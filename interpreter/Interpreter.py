import sys
import math
from antlr4 import *
from PseudocodeLexer import PseudocodeLexer
from PseudocodeParser import PseudocodeParser
from PseudocodeVisitor import PseudocodeVisitor

class Interpreter(PseudocodeVisitor):
    def __init__(self, debug=False):
        self.env = {}
        self.debug = debug

    def debug_print(self, msg):
        if self.debug:
            print(f"DEBUG: {msg}")

    def visit(self, node):
        """Override visit to add expression debugging"""
        if hasattr(node, 'getText') and self.debug:
            text = node.getText()
            if '*' in text and len(text) < 10:  # Only debug short expressions with *
                self.debug_print(f"Visiting expression: {text}")
        
        result = super().visit(node)
        
        if hasattr(node, 'getText') and self.debug:
            text = node.getText()
            if '*' in text and len(text) < 10:
                self.debug_print(f"Expression '{text}' evaluated to: {result}")
        
        return result

    def visitProgram(self, ctx):
        """Visit all statements in the program"""
        for stmt in ctx.stmt():
            self.visit(stmt)

    def visitAssign(self, ctx):
        """Handle assignment: NAME '<-' expr"""
        name = ctx.NAME().getText()
        value = self.visit(ctx.expr())
        self.env[name] = value
        self.debug_print(f"Assignment: {name} = {value}")

    def visitSwapStmt(self, ctx):
        """Handle swap statement: NAME '<->' NAME"""
        name1 = ctx.NAME(0).getText()
        name2 = ctx.NAME(1).getText()
        
        # Get current values (default to 0 if not found)
        val1 = self.env.get(name1, 0)
        val2 = self.env.get(name2, 0)
        
        # Swap the values
        self.env[name1] = val2
        self.env[name2] = val1
        
        self.debug_print(f"Swap: {name1} ({val1}) <-> {name2} ({val2})")

    def visitMultiStmt(self, ctx):
        """Handle multiple statements on the same line separated by semicolons"""
        # Get all children that are not semicolons
        for child in ctx.children:
            if hasattr(child, 'getText') and child.getText() != ';':
                self.visit(child)

    def visitReadStmt(self, ctx):
        """Handle read statement: 'citeste' nameList"""
        for name_token in ctx.nameList().NAME():
            name = name_token.getText()
            val = input(f"{name} = ")
            # Try to convert to number
            try:
                if '.' in val:
                    val = float(val)
                else:
                    val = int(val)
            except ValueError:
                # Keep as string if conversion fails
                pass
            self.env[name] = val
            self.debug_print(f"Read: {name} = {val}")

    def visitWriteStmt(self, ctx):
        """Handle write statement: 'scrie' exprList"""
        values = []
        for expr in ctx.exprList().expr():
            value = self.visit(expr)
            values.append(str(value))
        print(''.join(values), end="")

    def visitIfStmt(self, ctx):
        """Handle if statement"""
        condition = self.visit(ctx.expr())
        self.debug_print(f"If condition: {condition}")
        
        if condition:
            # Execute then block
            if ctx.thenBlock():
                self.debug_print("Executing then block")
                self.visit(ctx.thenBlock())
        else:
            # Execute else block if it exists
            if ctx.elseBlock():
                self.debug_print("Executing else block")
                self.visit(ctx.elseBlock())

    def visitThenBlock(self, ctx):
        """Handle then block - execute all statements"""
        for stmt in ctx.stmt():
            self.visit(stmt)

    def visitElseBlock(self, ctx):
        """Handle else block - execute all statements"""
        for stmt in ctx.stmt():
            self.visit(stmt)

    def visitForStmt(self, ctx):
        """Handle for loop: 'pentru' NAME '<-' expr ',' expr (',' expr)? 'executa' stmt* 'sf'"""
        var_name = ctx.NAME().getText()
        start_val = int(self.visit(ctx.expr(0)))
        end_val = int(self.visit(ctx.expr(1)))
        step_val = int(self.visit(ctx.expr(2))) if len(ctx.expr()) > 2 else 1
        
        self.debug_print(f"For loop: {var_name} from {start_val} to {end_val} step {step_val}")
        
        # Custom loop logic instead of Python's range
        current = start_val
        iteration_count = 0
        
        while True:
            # Check termination condition based on step direction
            if step_val > 0 and current > end_val:
                break
            elif step_val < 0 and current < end_val:
                break
            elif step_val == 0:
                raise ValueError("Step cannot be zero in for loop")
            
            iteration_count += 1
            self.env[var_name] = current
            self.debug_print(f"=== FOR LOOP iteration {iteration_count}: {var_name} = {current} ===")
            
            # Execute loop body
            for stmt in ctx.stmt():
                self.visit(stmt)
            
            # Update loop variable
            current += step_val
            
            # Safety check to prevent infinite loops
            if self.debug and iteration_count > 10000:
                print("DEBUG: Breaking potential infinite for loop")
                break

    def visitWhileStmt(self, ctx):
        """Handle while loop: 'cat' 'timp' expr 'executa' stmt* 'sf'"""
        iteration = 0
        while self.visit(ctx.expr()):
            iteration += 1
            self.debug_print(f"While loop iteration {iteration}")
            for stmt in ctx.stmt():
                self.visit(stmt)
            # Safety check to prevent infinite loops in debug mode
            if self.debug and iteration > 1000:
                print("DEBUG: Breaking potential infinite loop")
                break

    def visitDoWhileStmt(self, ctx):
        """Handle do-while loop: 'executa' stmt* 'cat' 'timp' expr"""
        iteration = 0
        while True:
            iteration += 1
            self.debug_print(f"Do-while loop iteration {iteration}")
            for stmt in ctx.stmt():
                self.visit(stmt)
            if not self.visit(ctx.expr()):
                break
            # Safety check
            if self.debug and iteration > 1000:
                print("DEBUG: Breaking potential infinite loop")
                break

    def visitRepeatStmt(self, ctx):
        """Handle repeat-until loop: 'repeta' stmt* 'pana' 'cand' expr"""
        iteration = 0
        while True:
            iteration += 1
            self.debug_print(f"Repeat loop iteration {iteration}")
            # Execute statements first
            for stmt in ctx.stmt():
                self.visit(stmt)
            # Then check condition
            if self.visit(ctx.expr()):
                self.debug_print("Repeat loop condition met, breaking")
                break
            # Safety check
            if self.debug and iteration > 1000:
                print("DEBUG: Breaking potential infinite loop")
                break

    # Expression visitors
    def visitOrExpr(self, ctx):
        """Handle OR expression: expr 'SAU' expr"""
        left = self.visit(ctx.expr(0))
        # Short-circuit evaluation
        if left:
            return True
        return bool(self.visit(ctx.expr(1)))

    def visitAndExpr(self, ctx):
        """Handle AND expression: expr 'SI' expr"""
        left = self.visit(ctx.expr(0))
        # Short-circuit evaluation
        if not left:
            return False
        return bool(self.visit(ctx.expr(1)))

    def visitNotExpr(self, ctx):
        """Handle NOT expression: 'NOT' expr"""
        return not bool(self.visit(ctx.expr()))

    def visitCompareExpr(self, ctx):
        """Handle comparison expressions"""
        left = self.visit(ctx.expr(0))
        right = self.visit(ctx.expr(1))
        op = ctx.getChild(1).getText()

        self.debug_print(f"Comparison: {left} {op} {right}")

        if op == "=":
            return left == right
        elif op == "!=":
            return left != right
        elif op == "<":
            return left < right
        elif op == "<=":
            return left <= right
        elif op == ">":
            return left > right
        elif op == ">=":
            return left >= right
        else:
            raise ValueError(f"Unknown comparison operator: {op}")

    def visitAddExpr(self, ctx):
        """Handle addition/subtraction expressions"""
        left = self.visit(ctx.expr(0))
        right = self.visit(ctx.expr(1))
        op = ctx.getChild(1).getText()
        
        if op == "+":
            return left + right
        elif op == "-":
            return left - right
        else:
            raise ValueError(f"Unknown additive operator: {op}")

    def visitMulExpr(self, ctx):
        """Handle multiplication/division/modulo expressions"""
        left = self.visit(ctx.expr(0))
        right = self.visit(ctx.expr(1))
        op = ctx.getChild(1).getText()
        
        self.debug_print(f"Multiplication: {left} {op} {right}")
        
        if op == "*":
            result = left * right
            self.debug_print(f"Multiplication result: {result}")
            return result
        elif op == "/":
            return left / right
        elif op == "%":
            return left % right
        else:
            raise ValueError(f"Unknown multiplicative operator: {op}")

    def visitIntDivExpr(self, ctx):
        """Handle integer division: '[' expr '/' expr ']'"""
        dividend = self.visit(ctx.expr(0))
        divisor = self.visit(ctx.expr(1))
        return int(dividend // divisor)

    def visitSqrtExpr(self, ctx):
        """Handle square root: '√' atom"""
        value = self.visit(ctx.atom())
        if value < 0:
            raise ValueError("Cannot take square root of negative number")
        result = math.sqrt(value)
        self.debug_print(f"Square root of {value} = {result}")
        return result

    def visitNegExpr(self, ctx):
        """Handle negation: '-' atom"""
        return -self.visit(ctx.atom())

    def visitAtomExpr(self, ctx):
        """Handle atomic expressions"""
        return self.visit(ctx.atom())

    def visitAtom(self, ctx):
        """Handle atomic values: numbers, strings, variables, parenthesized expressions"""
        if ctx.NUMBER():
            text = ctx.NUMBER().getText()
            if '.' in text:
                return float(text)
            else:
                return int(text)
        elif ctx.STRING():
            # Remove quotes
            text = ctx.STRING().getText()
            return text[1:-1]  # Remove first and last character (quotes)
        elif ctx.NAME():
            name = ctx.NAME().getText()
            value = self.env.get(name, 0)  # Default to 0 if variable not found
            self.debug_print(f"Variable access: {name} = {value}")
            return value
        elif ctx.expr():
            # Parenthesized expression
            return self.visit(ctx.expr())
        else:
            raise ValueError("Unknown atom type")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 interpreter.py <pseudocode_file> [--debug]")
        sys.exit(1)
    
    debug = "--debug" in sys.argv
    filename = sys.argv[1]
    
    try:
        input_stream = FileStream(filename, encoding='utf-8')
        lexer = PseudocodeLexer(input_stream)
        stream = CommonTokenStream(lexer)
        parser = PseudocodeParser(stream)
        tree = parser.program()
        
        interpreter = Interpreter(debug=debug)
        interpreter.visit(tree)
        
    except Exception as e:
        print(f"Error: {e}")
        if debug:
            import traceback
            traceback.print_exc()

if __name__ == '__main__':
    main()
    print()
