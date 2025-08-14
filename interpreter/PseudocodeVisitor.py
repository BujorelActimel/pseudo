# Generated from interpreter/Pseudocode.g4 by ANTLR 4.13.2
from antlr4 import *
if "." in __name__:
    from .PseudocodeParser import PseudocodeParser
else:
    from PseudocodeParser import PseudocodeParser

# This class defines a complete generic visitor for a parse tree produced by PseudocodeParser.

class PseudocodeVisitor(ParseTreeVisitor):

    # Visit a parse tree produced by PseudocodeParser#program.
    def visitProgram(self, ctx:PseudocodeParser.ProgramContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#stmt.
    def visitStmt(self, ctx:PseudocodeParser.StmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#assign.
    def visitAssign(self, ctx:PseudocodeParser.AssignContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#swapStmt.
    def visitSwapStmt(self, ctx:PseudocodeParser.SwapStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#multiStmt.
    def visitMultiStmt(self, ctx:PseudocodeParser.MultiStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#readStmt.
    def visitReadStmt(self, ctx:PseudocodeParser.ReadStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#writeStmt.
    def visitWriteStmt(self, ctx:PseudocodeParser.WriteStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#nameList.
    def visitNameList(self, ctx:PseudocodeParser.NameListContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#exprList.
    def visitExprList(self, ctx:PseudocodeParser.ExprListContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#ifStmt.
    def visitIfStmt(self, ctx:PseudocodeParser.IfStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#thenBlock.
    def visitThenBlock(self, ctx:PseudocodeParser.ThenBlockContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#elseBlock.
    def visitElseBlock(self, ctx:PseudocodeParser.ElseBlockContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#forStmt.
    def visitForStmt(self, ctx:PseudocodeParser.ForStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#whileStmt.
    def visitWhileStmt(self, ctx:PseudocodeParser.WhileStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#doWhileStmt.
    def visitDoWhileStmt(self, ctx:PseudocodeParser.DoWhileStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#repeatStmt.
    def visitRepeatStmt(self, ctx:PseudocodeParser.RepeatStmtContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#notExpr.
    def visitNotExpr(self, ctx:PseudocodeParser.NotExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#addExpr.
    def visitAddExpr(self, ctx:PseudocodeParser.AddExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#negExpr.
    def visitNegExpr(self, ctx:PseudocodeParser.NegExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#sqrtExpr.
    def visitSqrtExpr(self, ctx:PseudocodeParser.SqrtExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#mulExpr.
    def visitMulExpr(self, ctx:PseudocodeParser.MulExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#atomExpr.
    def visitAtomExpr(self, ctx:PseudocodeParser.AtomExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#orExpr.
    def visitOrExpr(self, ctx:PseudocodeParser.OrExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#int.
    def visitInt(self, ctx:PseudocodeParser.IntContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#compareExpr.
    def visitCompareExpr(self, ctx:PseudocodeParser.CompareExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#andExpr.
    def visitAndExpr(self, ctx:PseudocodeParser.AndExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by PseudocodeParser#atom.
    def visitAtom(self, ctx:PseudocodeParser.AtomContext):
        return self.visitChildren(ctx)



del PseudocodeParser