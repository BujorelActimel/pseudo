# Generated from Pseudocode.g4 by ANTLR 4.13.2
from antlr4 import *
if "." in __name__:
    from .PseudocodeParser import PseudocodeParser
else:
    from PseudocodeParser import PseudocodeParser

# This class defines a complete listener for a parse tree produced by PseudocodeParser.
class PseudocodeListener(ParseTreeListener):

    # Enter a parse tree produced by PseudocodeParser#program.
    def enterProgram(self, ctx:PseudocodeParser.ProgramContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#program.
    def exitProgram(self, ctx:PseudocodeParser.ProgramContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#stmt.
    def enterStmt(self, ctx:PseudocodeParser.StmtContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#stmt.
    def exitStmt(self, ctx:PseudocodeParser.StmtContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#assign.
    def enterAssign(self, ctx:PseudocodeParser.AssignContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#assign.
    def exitAssign(self, ctx:PseudocodeParser.AssignContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#readStmt.
    def enterReadStmt(self, ctx:PseudocodeParser.ReadStmtContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#readStmt.
    def exitReadStmt(self, ctx:PseudocodeParser.ReadStmtContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#writeStmt.
    def enterWriteStmt(self, ctx:PseudocodeParser.WriteStmtContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#writeStmt.
    def exitWriteStmt(self, ctx:PseudocodeParser.WriteStmtContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#nameList.
    def enterNameList(self, ctx:PseudocodeParser.NameListContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#nameList.
    def exitNameList(self, ctx:PseudocodeParser.NameListContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#exprList.
    def enterExprList(self, ctx:PseudocodeParser.ExprListContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#exprList.
    def exitExprList(self, ctx:PseudocodeParser.ExprListContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#ifStmt.
    def enterIfStmt(self, ctx:PseudocodeParser.IfStmtContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#ifStmt.
    def exitIfStmt(self, ctx:PseudocodeParser.IfStmtContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#thenBlock.
    def enterThenBlock(self, ctx:PseudocodeParser.ThenBlockContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#thenBlock.
    def exitThenBlock(self, ctx:PseudocodeParser.ThenBlockContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#elseBlock.
    def enterElseBlock(self, ctx:PseudocodeParser.ElseBlockContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#elseBlock.
    def exitElseBlock(self, ctx:PseudocodeParser.ElseBlockContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#forStmt.
    def enterForStmt(self, ctx:PseudocodeParser.ForStmtContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#forStmt.
    def exitForStmt(self, ctx:PseudocodeParser.ForStmtContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#whileStmt.
    def enterWhileStmt(self, ctx:PseudocodeParser.WhileStmtContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#whileStmt.
    def exitWhileStmt(self, ctx:PseudocodeParser.WhileStmtContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#doWhileStmt.
    def enterDoWhileStmt(self, ctx:PseudocodeParser.DoWhileStmtContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#doWhileStmt.
    def exitDoWhileStmt(self, ctx:PseudocodeParser.DoWhileStmtContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#repeatStmt.
    def enterRepeatStmt(self, ctx:PseudocodeParser.RepeatStmtContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#repeatStmt.
    def exitRepeatStmt(self, ctx:PseudocodeParser.RepeatStmtContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#intDivExpr.
    def enterIntDivExpr(self, ctx:PseudocodeParser.IntDivExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#intDivExpr.
    def exitIntDivExpr(self, ctx:PseudocodeParser.IntDivExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#notExpr.
    def enterNotExpr(self, ctx:PseudocodeParser.NotExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#notExpr.
    def exitNotExpr(self, ctx:PseudocodeParser.NotExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#addExpr.
    def enterAddExpr(self, ctx:PseudocodeParser.AddExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#addExpr.
    def exitAddExpr(self, ctx:PseudocodeParser.AddExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#negExpr.
    def enterNegExpr(self, ctx:PseudocodeParser.NegExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#negExpr.
    def exitNegExpr(self, ctx:PseudocodeParser.NegExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#mulExpr.
    def enterMulExpr(self, ctx:PseudocodeParser.MulExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#mulExpr.
    def exitMulExpr(self, ctx:PseudocodeParser.MulExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#atomExpr.
    def enterAtomExpr(self, ctx:PseudocodeParser.AtomExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#atomExpr.
    def exitAtomExpr(self, ctx:PseudocodeParser.AtomExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#orExpr.
    def enterOrExpr(self, ctx:PseudocodeParser.OrExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#orExpr.
    def exitOrExpr(self, ctx:PseudocodeParser.OrExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#compareExpr.
    def enterCompareExpr(self, ctx:PseudocodeParser.CompareExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#compareExpr.
    def exitCompareExpr(self, ctx:PseudocodeParser.CompareExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#andExpr.
    def enterAndExpr(self, ctx:PseudocodeParser.AndExprContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#andExpr.
    def exitAndExpr(self, ctx:PseudocodeParser.AndExprContext):
        pass


    # Enter a parse tree produced by PseudocodeParser#atom.
    def enterAtom(self, ctx:PseudocodeParser.AtomContext):
        pass

    # Exit a parse tree produced by PseudocodeParser#atom.
    def exitAtom(self, ctx:PseudocodeParser.AtomContext):
        pass



del PseudocodeParser