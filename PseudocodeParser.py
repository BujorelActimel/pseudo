# Generated from Pseudocode.g4 by ANTLR 4.13.2
# encoding: utf-8
from antlr4 import *
from io import StringIO
import sys
if sys.version_info[1] > 5:
	from typing import TextIO
else:
	from typing.io import TextIO

def serializedATN():
    return [
        4,1,38,195,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
        6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,
        2,14,7,14,2,15,7,15,1,0,5,0,34,8,0,10,0,12,0,37,9,0,1,0,1,0,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,49,8,1,1,2,1,2,1,2,1,2,1,3,1,3,1,
        3,1,4,1,4,1,4,1,5,1,5,1,5,5,5,64,8,5,10,5,12,5,67,9,5,1,6,1,6,1,
        6,5,6,72,8,6,10,6,12,6,75,9,6,1,7,1,7,1,7,1,7,1,7,1,7,3,7,83,8,7,
        1,7,1,7,1,8,5,8,88,8,8,10,8,12,8,91,9,8,1,9,5,9,94,8,9,10,9,12,9,
        97,9,9,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,3,10,107,8,10,1,10,
        1,10,5,10,111,8,10,10,10,12,10,114,9,10,1,10,1,10,1,11,1,11,1,11,
        1,11,1,11,5,11,123,8,11,10,11,12,11,126,9,11,1,11,1,11,1,12,1,12,
        5,12,132,8,12,10,12,12,12,135,9,12,1,12,1,12,1,12,1,12,1,13,1,13,
        5,13,143,8,13,10,13,12,13,146,9,13,1,13,1,13,1,13,1,13,1,14,1,14,
        1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,3,14,164,8,14,
        1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,
        1,14,1,14,5,14,181,8,14,10,14,12,14,184,9,14,1,15,1,15,1,15,1,15,
        1,15,1,15,1,15,3,15,193,8,15,1,15,0,1,28,16,0,2,4,6,8,10,12,14,16,
        18,20,22,24,26,28,30,0,3,1,0,16,18,1,0,21,22,1,0,23,28,207,0,35,
        1,0,0,0,2,48,1,0,0,0,4,50,1,0,0,0,6,54,1,0,0,0,8,57,1,0,0,0,10,60,
        1,0,0,0,12,68,1,0,0,0,14,76,1,0,0,0,16,89,1,0,0,0,18,95,1,0,0,0,
        20,98,1,0,0,0,22,117,1,0,0,0,24,129,1,0,0,0,26,140,1,0,0,0,28,163,
        1,0,0,0,30,192,1,0,0,0,32,34,3,2,1,0,33,32,1,0,0,0,34,37,1,0,0,0,
        35,33,1,0,0,0,35,36,1,0,0,0,36,38,1,0,0,0,37,35,1,0,0,0,38,39,5,
        0,0,1,39,1,1,0,0,0,40,49,3,4,2,0,41,49,3,6,3,0,42,49,3,8,4,0,43,
        49,3,14,7,0,44,49,3,20,10,0,45,49,3,22,11,0,46,49,3,24,12,0,47,49,
        3,26,13,0,48,40,1,0,0,0,48,41,1,0,0,0,48,42,1,0,0,0,48,43,1,0,0,
        0,48,44,1,0,0,0,48,45,1,0,0,0,48,46,1,0,0,0,48,47,1,0,0,0,49,3,1,
        0,0,0,50,51,5,34,0,0,51,52,5,1,0,0,52,53,3,28,14,0,53,5,1,0,0,0,
        54,55,5,2,0,0,55,56,3,10,5,0,56,7,1,0,0,0,57,58,5,3,0,0,58,59,3,
        12,6,0,59,9,1,0,0,0,60,65,5,34,0,0,61,62,5,4,0,0,62,64,5,34,0,0,
        63,61,1,0,0,0,64,67,1,0,0,0,65,63,1,0,0,0,65,66,1,0,0,0,66,11,1,
        0,0,0,67,65,1,0,0,0,68,73,3,28,14,0,69,70,5,4,0,0,70,72,3,28,14,
        0,71,69,1,0,0,0,72,75,1,0,0,0,73,71,1,0,0,0,73,74,1,0,0,0,74,13,
        1,0,0,0,75,73,1,0,0,0,76,77,5,5,0,0,77,78,3,28,14,0,78,79,5,6,0,
        0,79,82,3,16,8,0,80,81,5,7,0,0,81,83,3,18,9,0,82,80,1,0,0,0,82,83,
        1,0,0,0,83,84,1,0,0,0,84,85,5,8,0,0,85,15,1,0,0,0,86,88,3,2,1,0,
        87,86,1,0,0,0,88,91,1,0,0,0,89,87,1,0,0,0,89,90,1,0,0,0,90,17,1,
        0,0,0,91,89,1,0,0,0,92,94,3,2,1,0,93,92,1,0,0,0,94,97,1,0,0,0,95,
        93,1,0,0,0,95,96,1,0,0,0,96,19,1,0,0,0,97,95,1,0,0,0,98,99,5,9,0,
        0,99,100,5,34,0,0,100,101,5,1,0,0,101,102,3,28,14,0,102,103,5,4,
        0,0,103,106,3,28,14,0,104,105,5,4,0,0,105,107,3,28,14,0,106,104,
        1,0,0,0,106,107,1,0,0,0,107,108,1,0,0,0,108,112,5,10,0,0,109,111,
        3,2,1,0,110,109,1,0,0,0,111,114,1,0,0,0,112,110,1,0,0,0,112,113,
        1,0,0,0,113,115,1,0,0,0,114,112,1,0,0,0,115,116,5,8,0,0,116,21,1,
        0,0,0,117,118,5,11,0,0,118,119,5,12,0,0,119,120,3,28,14,0,120,124,
        5,10,0,0,121,123,3,2,1,0,122,121,1,0,0,0,123,126,1,0,0,0,124,122,
        1,0,0,0,124,125,1,0,0,0,125,127,1,0,0,0,126,124,1,0,0,0,127,128,
        5,8,0,0,128,23,1,0,0,0,129,133,5,10,0,0,130,132,3,2,1,0,131,130,
        1,0,0,0,132,135,1,0,0,0,133,131,1,0,0,0,133,134,1,0,0,0,134,136,
        1,0,0,0,135,133,1,0,0,0,136,137,5,11,0,0,137,138,5,12,0,0,138,139,
        3,28,14,0,139,25,1,0,0,0,140,144,5,13,0,0,141,143,3,2,1,0,142,141,
        1,0,0,0,143,146,1,0,0,0,144,142,1,0,0,0,144,145,1,0,0,0,145,147,
        1,0,0,0,146,144,1,0,0,0,147,148,5,14,0,0,148,149,5,15,0,0,149,150,
        3,28,14,0,150,27,1,0,0,0,151,152,6,14,-1,0,152,153,5,19,0,0,153,
        154,3,28,14,0,154,155,5,17,0,0,155,156,3,28,14,0,156,157,5,20,0,
        0,157,164,1,0,0,0,158,159,5,22,0,0,159,164,3,28,14,6,160,164,3,30,
        15,0,161,162,5,31,0,0,162,164,3,28,14,1,163,151,1,0,0,0,163,158,
        1,0,0,0,163,160,1,0,0,0,163,161,1,0,0,0,164,182,1,0,0,0,165,166,
        10,9,0,0,166,167,7,0,0,0,167,181,3,28,14,10,168,169,10,7,0,0,169,
        170,7,1,0,0,170,181,3,28,14,8,171,172,10,5,0,0,172,173,7,2,0,0,173,
        181,3,28,14,6,174,175,10,3,0,0,175,176,5,29,0,0,176,181,3,28,14,
        4,177,178,10,2,0,0,178,179,5,30,0,0,179,181,3,28,14,3,180,165,1,
        0,0,0,180,168,1,0,0,0,180,171,1,0,0,0,180,174,1,0,0,0,180,177,1,
        0,0,0,181,184,1,0,0,0,182,180,1,0,0,0,182,183,1,0,0,0,183,29,1,0,
        0,0,184,182,1,0,0,0,185,193,5,35,0,0,186,193,5,36,0,0,187,193,5,
        34,0,0,188,189,5,32,0,0,189,190,3,28,14,0,190,191,5,33,0,0,191,193,
        1,0,0,0,192,185,1,0,0,0,192,186,1,0,0,0,192,187,1,0,0,0,192,188,
        1,0,0,0,193,31,1,0,0,0,16,35,48,65,73,82,89,95,106,112,124,133,144,
        163,180,182,192
    ]

class PseudocodeParser ( Parser ):

    grammarFileName = "Pseudocode.g4"

    atn = ATNDeserializer().deserialize(serializedATN())

    decisionsToDFA = [ DFA(ds, i) for i, ds in enumerate(atn.decisionToState) ]

    sharedContextCache = PredictionContextCache()

    literalNames = [ "<INVALID>", "'<-'", "'citeste'", "'scrie'", "','", 
                     "'daca'", "'atunci'", "'altfel'", "'sf'", "'pentru'", 
                     "'executa'", "'cat'", "'timp'", "'repeta'", "'pana'", 
                     "'cand'", "'*'", "'/'", "'%'", "'['", "']'", "'+'", 
                     "'-'", "'='", "'!='", "'<'", "'<='", "'>'", "'>='", 
                     "'SAU'", "'SI'", "'NOT'", "'('", "')'" ]

    symbolicNames = [ "<INVALID>", "<INVALID>", "<INVALID>", "<INVALID>", 
                      "<INVALID>", "<INVALID>", "<INVALID>", "<INVALID>", 
                      "<INVALID>", "<INVALID>", "<INVALID>", "<INVALID>", 
                      "<INVALID>", "<INVALID>", "<INVALID>", "<INVALID>", 
                      "<INVALID>", "<INVALID>", "<INVALID>", "<INVALID>", 
                      "<INVALID>", "<INVALID>", "<INVALID>", "<INVALID>", 
                      "<INVALID>", "<INVALID>", "<INVALID>", "<INVALID>", 
                      "<INVALID>", "<INVALID>", "<INVALID>", "<INVALID>", 
                      "<INVALID>", "<INVALID>", "NAME", "NUMBER", "STRING", 
                      "WS", "COMMENT" ]

    RULE_program = 0
    RULE_stmt = 1
    RULE_assign = 2
    RULE_readStmt = 3
    RULE_writeStmt = 4
    RULE_nameList = 5
    RULE_exprList = 6
    RULE_ifStmt = 7
    RULE_thenBlock = 8
    RULE_elseBlock = 9
    RULE_forStmt = 10
    RULE_whileStmt = 11
    RULE_doWhileStmt = 12
    RULE_repeatStmt = 13
    RULE_expr = 14
    RULE_atom = 15

    ruleNames =  [ "program", "stmt", "assign", "readStmt", "writeStmt", 
                   "nameList", "exprList", "ifStmt", "thenBlock", "elseBlock", 
                   "forStmt", "whileStmt", "doWhileStmt", "repeatStmt", 
                   "expr", "atom" ]

    EOF = Token.EOF
    T__0=1
    T__1=2
    T__2=3
    T__3=4
    T__4=5
    T__5=6
    T__6=7
    T__7=8
    T__8=9
    T__9=10
    T__10=11
    T__11=12
    T__12=13
    T__13=14
    T__14=15
    T__15=16
    T__16=17
    T__17=18
    T__18=19
    T__19=20
    T__20=21
    T__21=22
    T__22=23
    T__23=24
    T__24=25
    T__25=26
    T__26=27
    T__27=28
    T__28=29
    T__29=30
    T__30=31
    T__31=32
    T__32=33
    NAME=34
    NUMBER=35
    STRING=36
    WS=37
    COMMENT=38

    def __init__(self, input:TokenStream, output:TextIO = sys.stdout):
        super().__init__(input, output)
        self.checkVersion("4.13.2")
        self._interp = ParserATNSimulator(self, self.atn, self.decisionsToDFA, self.sharedContextCache)
        self._predicates = None




    class ProgramContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def EOF(self):
            return self.getToken(PseudocodeParser.EOF, 0)

        def stmt(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.StmtContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.StmtContext,i)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_program

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterProgram" ):
                listener.enterProgram(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitProgram" ):
                listener.exitProgram(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitProgram" ):
                return visitor.visitProgram(self)
            else:
                return visitor.visitChildren(self)




    def program(self):

        localctx = PseudocodeParser.ProgramContext(self, self._ctx, self.state)
        self.enterRule(localctx, 0, self.RULE_program)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 35
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while (((_la) & ~0x3f) == 0 and ((1 << _la) & 17179881004) != 0):
                self.state = 32
                self.stmt()
                self.state = 37
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 38
            self.match(PseudocodeParser.EOF)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class StmtContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def assign(self):
            return self.getTypedRuleContext(PseudocodeParser.AssignContext,0)


        def readStmt(self):
            return self.getTypedRuleContext(PseudocodeParser.ReadStmtContext,0)


        def writeStmt(self):
            return self.getTypedRuleContext(PseudocodeParser.WriteStmtContext,0)


        def ifStmt(self):
            return self.getTypedRuleContext(PseudocodeParser.IfStmtContext,0)


        def forStmt(self):
            return self.getTypedRuleContext(PseudocodeParser.ForStmtContext,0)


        def whileStmt(self):
            return self.getTypedRuleContext(PseudocodeParser.WhileStmtContext,0)


        def doWhileStmt(self):
            return self.getTypedRuleContext(PseudocodeParser.DoWhileStmtContext,0)


        def repeatStmt(self):
            return self.getTypedRuleContext(PseudocodeParser.RepeatStmtContext,0)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_stmt

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterStmt" ):
                listener.enterStmt(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitStmt" ):
                listener.exitStmt(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitStmt" ):
                return visitor.visitStmt(self)
            else:
                return visitor.visitChildren(self)




    def stmt(self):

        localctx = PseudocodeParser.StmtContext(self, self._ctx, self.state)
        self.enterRule(localctx, 2, self.RULE_stmt)
        try:
            self.state = 48
            self._errHandler.sync(self)
            token = self._input.LA(1)
            if token in [34]:
                self.enterOuterAlt(localctx, 1)
                self.state = 40
                self.assign()
                pass
            elif token in [2]:
                self.enterOuterAlt(localctx, 2)
                self.state = 41
                self.readStmt()
                pass
            elif token in [3]:
                self.enterOuterAlt(localctx, 3)
                self.state = 42
                self.writeStmt()
                pass
            elif token in [5]:
                self.enterOuterAlt(localctx, 4)
                self.state = 43
                self.ifStmt()
                pass
            elif token in [9]:
                self.enterOuterAlt(localctx, 5)
                self.state = 44
                self.forStmt()
                pass
            elif token in [11]:
                self.enterOuterAlt(localctx, 6)
                self.state = 45
                self.whileStmt()
                pass
            elif token in [10]:
                self.enterOuterAlt(localctx, 7)
                self.state = 46
                self.doWhileStmt()
                pass
            elif token in [13]:
                self.enterOuterAlt(localctx, 8)
                self.state = 47
                self.repeatStmt()
                pass
            else:
                raise NoViableAltException(self)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class AssignContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(PseudocodeParser.NAME, 0)

        def expr(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprContext,0)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_assign

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterAssign" ):
                listener.enterAssign(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitAssign" ):
                listener.exitAssign(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitAssign" ):
                return visitor.visitAssign(self)
            else:
                return visitor.visitChildren(self)




    def assign(self):

        localctx = PseudocodeParser.AssignContext(self, self._ctx, self.state)
        self.enterRule(localctx, 4, self.RULE_assign)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 50
            self.match(PseudocodeParser.NAME)
            self.state = 51
            self.match(PseudocodeParser.T__0)
            self.state = 52
            self.expr(0)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class ReadStmtContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def nameList(self):
            return self.getTypedRuleContext(PseudocodeParser.NameListContext,0)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_readStmt

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterReadStmt" ):
                listener.enterReadStmt(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitReadStmt" ):
                listener.exitReadStmt(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitReadStmt" ):
                return visitor.visitReadStmt(self)
            else:
                return visitor.visitChildren(self)




    def readStmt(self):

        localctx = PseudocodeParser.ReadStmtContext(self, self._ctx, self.state)
        self.enterRule(localctx, 6, self.RULE_readStmt)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 54
            self.match(PseudocodeParser.T__1)
            self.state = 55
            self.nameList()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class WriteStmtContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def exprList(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprListContext,0)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_writeStmt

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterWriteStmt" ):
                listener.enterWriteStmt(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitWriteStmt" ):
                listener.exitWriteStmt(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitWriteStmt" ):
                return visitor.visitWriteStmt(self)
            else:
                return visitor.visitChildren(self)




    def writeStmt(self):

        localctx = PseudocodeParser.WriteStmtContext(self, self._ctx, self.state)
        self.enterRule(localctx, 8, self.RULE_writeStmt)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 57
            self.match(PseudocodeParser.T__2)
            self.state = 58
            self.exprList()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class NameListContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self, i:int=None):
            if i is None:
                return self.getTokens(PseudocodeParser.NAME)
            else:
                return self.getToken(PseudocodeParser.NAME, i)

        def getRuleIndex(self):
            return PseudocodeParser.RULE_nameList

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterNameList" ):
                listener.enterNameList(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitNameList" ):
                listener.exitNameList(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitNameList" ):
                return visitor.visitNameList(self)
            else:
                return visitor.visitChildren(self)




    def nameList(self):

        localctx = PseudocodeParser.NameListContext(self, self._ctx, self.state)
        self.enterRule(localctx, 10, self.RULE_nameList)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 60
            self.match(PseudocodeParser.NAME)
            self.state = 65
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==4:
                self.state = 61
                self.match(PseudocodeParser.T__3)
                self.state = 62
                self.match(PseudocodeParser.NAME)
                self.state = 67
                self._errHandler.sync(self)
                _la = self._input.LA(1)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class ExprListContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def expr(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.ExprContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.ExprContext,i)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_exprList

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterExprList" ):
                listener.enterExprList(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitExprList" ):
                listener.exitExprList(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitExprList" ):
                return visitor.visitExprList(self)
            else:
                return visitor.visitChildren(self)




    def exprList(self):

        localctx = PseudocodeParser.ExprListContext(self, self._ctx, self.state)
        self.enterRule(localctx, 12, self.RULE_exprList)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 68
            self.expr(0)
            self.state = 73
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==4:
                self.state = 69
                self.match(PseudocodeParser.T__3)
                self.state = 70
                self.expr(0)
                self.state = 75
                self._errHandler.sync(self)
                _la = self._input.LA(1)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class IfStmtContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def expr(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprContext,0)


        def thenBlock(self):
            return self.getTypedRuleContext(PseudocodeParser.ThenBlockContext,0)


        def elseBlock(self):
            return self.getTypedRuleContext(PseudocodeParser.ElseBlockContext,0)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_ifStmt

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterIfStmt" ):
                listener.enterIfStmt(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitIfStmt" ):
                listener.exitIfStmt(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitIfStmt" ):
                return visitor.visitIfStmt(self)
            else:
                return visitor.visitChildren(self)




    def ifStmt(self):

        localctx = PseudocodeParser.IfStmtContext(self, self._ctx, self.state)
        self.enterRule(localctx, 14, self.RULE_ifStmt)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 76
            self.match(PseudocodeParser.T__4)
            self.state = 77
            self.expr(0)
            self.state = 78
            self.match(PseudocodeParser.T__5)
            self.state = 79
            self.thenBlock()
            self.state = 82
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            if _la==7:
                self.state = 80
                self.match(PseudocodeParser.T__6)
                self.state = 81
                self.elseBlock()


            self.state = 84
            self.match(PseudocodeParser.T__7)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class ThenBlockContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def stmt(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.StmtContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.StmtContext,i)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_thenBlock

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterThenBlock" ):
                listener.enterThenBlock(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitThenBlock" ):
                listener.exitThenBlock(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitThenBlock" ):
                return visitor.visitThenBlock(self)
            else:
                return visitor.visitChildren(self)




    def thenBlock(self):

        localctx = PseudocodeParser.ThenBlockContext(self, self._ctx, self.state)
        self.enterRule(localctx, 16, self.RULE_thenBlock)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 89
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while (((_la) & ~0x3f) == 0 and ((1 << _la) & 17179881004) != 0):
                self.state = 86
                self.stmt()
                self.state = 91
                self._errHandler.sync(self)
                _la = self._input.LA(1)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class ElseBlockContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def stmt(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.StmtContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.StmtContext,i)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_elseBlock

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterElseBlock" ):
                listener.enterElseBlock(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitElseBlock" ):
                listener.exitElseBlock(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitElseBlock" ):
                return visitor.visitElseBlock(self)
            else:
                return visitor.visitChildren(self)




    def elseBlock(self):

        localctx = PseudocodeParser.ElseBlockContext(self, self._ctx, self.state)
        self.enterRule(localctx, 18, self.RULE_elseBlock)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 95
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while (((_la) & ~0x3f) == 0 and ((1 << _la) & 17179881004) != 0):
                self.state = 92
                self.stmt()
                self.state = 97
                self._errHandler.sync(self)
                _la = self._input.LA(1)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class ForStmtContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(PseudocodeParser.NAME, 0)

        def expr(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.ExprContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.ExprContext,i)


        def stmt(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.StmtContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.StmtContext,i)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_forStmt

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterForStmt" ):
                listener.enterForStmt(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitForStmt" ):
                listener.exitForStmt(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitForStmt" ):
                return visitor.visitForStmt(self)
            else:
                return visitor.visitChildren(self)




    def forStmt(self):

        localctx = PseudocodeParser.ForStmtContext(self, self._ctx, self.state)
        self.enterRule(localctx, 20, self.RULE_forStmt)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 98
            self.match(PseudocodeParser.T__8)
            self.state = 99
            self.match(PseudocodeParser.NAME)
            self.state = 100
            self.match(PseudocodeParser.T__0)
            self.state = 101
            self.expr(0)
            self.state = 102
            self.match(PseudocodeParser.T__3)
            self.state = 103
            self.expr(0)
            self.state = 106
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            if _la==4:
                self.state = 104
                self.match(PseudocodeParser.T__3)
                self.state = 105
                self.expr(0)


            self.state = 108
            self.match(PseudocodeParser.T__9)
            self.state = 112
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while (((_la) & ~0x3f) == 0 and ((1 << _la) & 17179881004) != 0):
                self.state = 109
                self.stmt()
                self.state = 114
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 115
            self.match(PseudocodeParser.T__7)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class WhileStmtContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def expr(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprContext,0)


        def stmt(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.StmtContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.StmtContext,i)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_whileStmt

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterWhileStmt" ):
                listener.enterWhileStmt(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitWhileStmt" ):
                listener.exitWhileStmt(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitWhileStmt" ):
                return visitor.visitWhileStmt(self)
            else:
                return visitor.visitChildren(self)




    def whileStmt(self):

        localctx = PseudocodeParser.WhileStmtContext(self, self._ctx, self.state)
        self.enterRule(localctx, 22, self.RULE_whileStmt)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 117
            self.match(PseudocodeParser.T__10)
            self.state = 118
            self.match(PseudocodeParser.T__11)
            self.state = 119
            self.expr(0)
            self.state = 120
            self.match(PseudocodeParser.T__9)
            self.state = 124
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while (((_la) & ~0x3f) == 0 and ((1 << _la) & 17179881004) != 0):
                self.state = 121
                self.stmt()
                self.state = 126
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 127
            self.match(PseudocodeParser.T__7)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class DoWhileStmtContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def expr(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprContext,0)


        def stmt(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.StmtContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.StmtContext,i)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_doWhileStmt

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterDoWhileStmt" ):
                listener.enterDoWhileStmt(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitDoWhileStmt" ):
                listener.exitDoWhileStmt(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitDoWhileStmt" ):
                return visitor.visitDoWhileStmt(self)
            else:
                return visitor.visitChildren(self)




    def doWhileStmt(self):

        localctx = PseudocodeParser.DoWhileStmtContext(self, self._ctx, self.state)
        self.enterRule(localctx, 24, self.RULE_doWhileStmt)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 129
            self.match(PseudocodeParser.T__9)
            self.state = 133
            self._errHandler.sync(self)
            _alt = self._interp.adaptivePredict(self._input,10,self._ctx)
            while _alt!=2 and _alt!=ATN.INVALID_ALT_NUMBER:
                if _alt==1:
                    self.state = 130
                    self.stmt() 
                self.state = 135
                self._errHandler.sync(self)
                _alt = self._interp.adaptivePredict(self._input,10,self._ctx)

            self.state = 136
            self.match(PseudocodeParser.T__10)
            self.state = 137
            self.match(PseudocodeParser.T__11)
            self.state = 138
            self.expr(0)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class RepeatStmtContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def expr(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprContext,0)


        def stmt(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.StmtContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.StmtContext,i)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_repeatStmt

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterRepeatStmt" ):
                listener.enterRepeatStmt(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitRepeatStmt" ):
                listener.exitRepeatStmt(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitRepeatStmt" ):
                return visitor.visitRepeatStmt(self)
            else:
                return visitor.visitChildren(self)




    def repeatStmt(self):

        localctx = PseudocodeParser.RepeatStmtContext(self, self._ctx, self.state)
        self.enterRule(localctx, 26, self.RULE_repeatStmt)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 140
            self.match(PseudocodeParser.T__12)
            self.state = 144
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while (((_la) & ~0x3f) == 0 and ((1 << _la) & 17179881004) != 0):
                self.state = 141
                self.stmt()
                self.state = 146
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 147
            self.match(PseudocodeParser.T__13)
            self.state = 148
            self.match(PseudocodeParser.T__14)
            self.state = 149
            self.expr(0)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx


    class ExprContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return PseudocodeParser.RULE_expr

     
        def copyFrom(self, ctx:ParserRuleContext):
            super().copyFrom(ctx)


    class IntDivExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def expr(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.ExprContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.ExprContext,i)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterIntDivExpr" ):
                listener.enterIntDivExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitIntDivExpr" ):
                listener.exitIntDivExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitIntDivExpr" ):
                return visitor.visitIntDivExpr(self)
            else:
                return visitor.visitChildren(self)


    class NotExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def expr(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprContext,0)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterNotExpr" ):
                listener.enterNotExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitNotExpr" ):
                listener.exitNotExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitNotExpr" ):
                return visitor.visitNotExpr(self)
            else:
                return visitor.visitChildren(self)


    class AddExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def expr(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.ExprContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.ExprContext,i)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterAddExpr" ):
                listener.enterAddExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitAddExpr" ):
                listener.exitAddExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitAddExpr" ):
                return visitor.visitAddExpr(self)
            else:
                return visitor.visitChildren(self)


    class NegExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def expr(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprContext,0)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterNegExpr" ):
                listener.enterNegExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitNegExpr" ):
                listener.exitNegExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitNegExpr" ):
                return visitor.visitNegExpr(self)
            else:
                return visitor.visitChildren(self)


    class MulExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def expr(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.ExprContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.ExprContext,i)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterMulExpr" ):
                listener.enterMulExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitMulExpr" ):
                listener.exitMulExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitMulExpr" ):
                return visitor.visitMulExpr(self)
            else:
                return visitor.visitChildren(self)


    class AtomExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def atom(self):
            return self.getTypedRuleContext(PseudocodeParser.AtomContext,0)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterAtomExpr" ):
                listener.enterAtomExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitAtomExpr" ):
                listener.exitAtomExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitAtomExpr" ):
                return visitor.visitAtomExpr(self)
            else:
                return visitor.visitChildren(self)


    class OrExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def expr(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.ExprContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.ExprContext,i)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterOrExpr" ):
                listener.enterOrExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitOrExpr" ):
                listener.exitOrExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitOrExpr" ):
                return visitor.visitOrExpr(self)
            else:
                return visitor.visitChildren(self)


    class CompareExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def expr(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.ExprContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.ExprContext,i)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterCompareExpr" ):
                listener.enterCompareExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitCompareExpr" ):
                listener.exitCompareExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitCompareExpr" ):
                return visitor.visitCompareExpr(self)
            else:
                return visitor.visitChildren(self)


    class AndExprContext(ExprContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a PseudocodeParser.ExprContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def expr(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(PseudocodeParser.ExprContext)
            else:
                return self.getTypedRuleContext(PseudocodeParser.ExprContext,i)


        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterAndExpr" ):
                listener.enterAndExpr(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitAndExpr" ):
                listener.exitAndExpr(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitAndExpr" ):
                return visitor.visitAndExpr(self)
            else:
                return visitor.visitChildren(self)



    def expr(self, _p:int=0):
        _parentctx = self._ctx
        _parentState = self.state
        localctx = PseudocodeParser.ExprContext(self, self._ctx, _parentState)
        _prevctx = localctx
        _startState = 28
        self.enterRecursionRule(localctx, 28, self.RULE_expr, _p)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 163
            self._errHandler.sync(self)
            token = self._input.LA(1)
            if token in [19]:
                localctx = PseudocodeParser.IntDivExprContext(self, localctx)
                self._ctx = localctx
                _prevctx = localctx

                self.state = 152
                self.match(PseudocodeParser.T__18)
                self.state = 153
                self.expr(0)
                self.state = 154
                self.match(PseudocodeParser.T__16)
                self.state = 155
                self.expr(0)
                self.state = 156
                self.match(PseudocodeParser.T__19)
                pass
            elif token in [22]:
                localctx = PseudocodeParser.NegExprContext(self, localctx)
                self._ctx = localctx
                _prevctx = localctx
                self.state = 158
                self.match(PseudocodeParser.T__21)
                self.state = 159
                self.expr(6)
                pass
            elif token in [32, 34, 35, 36]:
                localctx = PseudocodeParser.AtomExprContext(self, localctx)
                self._ctx = localctx
                _prevctx = localctx
                self.state = 160
                self.atom()
                pass
            elif token in [31]:
                localctx = PseudocodeParser.NotExprContext(self, localctx)
                self._ctx = localctx
                _prevctx = localctx
                self.state = 161
                self.match(PseudocodeParser.T__30)
                self.state = 162
                self.expr(1)
                pass
            else:
                raise NoViableAltException(self)

            self._ctx.stop = self._input.LT(-1)
            self.state = 182
            self._errHandler.sync(self)
            _alt = self._interp.adaptivePredict(self._input,14,self._ctx)
            while _alt!=2 and _alt!=ATN.INVALID_ALT_NUMBER:
                if _alt==1:
                    if self._parseListeners is not None:
                        self.triggerExitRuleEvent()
                    _prevctx = localctx
                    self.state = 180
                    self._errHandler.sync(self)
                    la_ = self._interp.adaptivePredict(self._input,13,self._ctx)
                    if la_ == 1:
                        localctx = PseudocodeParser.MulExprContext(self, PseudocodeParser.ExprContext(self, _parentctx, _parentState))
                        self.pushNewRecursionContext(localctx, _startState, self.RULE_expr)
                        self.state = 165
                        if not self.precpred(self._ctx, 9):
                            from antlr4.error.Errors import FailedPredicateException
                            raise FailedPredicateException(self, "self.precpred(self._ctx, 9)")
                        self.state = 166
                        _la = self._input.LA(1)
                        if not((((_la) & ~0x3f) == 0 and ((1 << _la) & 458752) != 0)):
                            self._errHandler.recoverInline(self)
                        else:
                            self._errHandler.reportMatch(self)
                            self.consume()
                        self.state = 167
                        self.expr(10)
                        pass

                    elif la_ == 2:
                        localctx = PseudocodeParser.AddExprContext(self, PseudocodeParser.ExprContext(self, _parentctx, _parentState))
                        self.pushNewRecursionContext(localctx, _startState, self.RULE_expr)
                        self.state = 168
                        if not self.precpred(self._ctx, 7):
                            from antlr4.error.Errors import FailedPredicateException
                            raise FailedPredicateException(self, "self.precpred(self._ctx, 7)")
                        self.state = 169
                        _la = self._input.LA(1)
                        if not(_la==21 or _la==22):
                            self._errHandler.recoverInline(self)
                        else:
                            self._errHandler.reportMatch(self)
                            self.consume()
                        self.state = 170
                        self.expr(8)
                        pass

                    elif la_ == 3:
                        localctx = PseudocodeParser.CompareExprContext(self, PseudocodeParser.ExprContext(self, _parentctx, _parentState))
                        self.pushNewRecursionContext(localctx, _startState, self.RULE_expr)
                        self.state = 171
                        if not self.precpred(self._ctx, 5):
                            from antlr4.error.Errors import FailedPredicateException
                            raise FailedPredicateException(self, "self.precpred(self._ctx, 5)")
                        self.state = 172
                        _la = self._input.LA(1)
                        if not((((_la) & ~0x3f) == 0 and ((1 << _la) & 528482304) != 0)):
                            self._errHandler.recoverInline(self)
                        else:
                            self._errHandler.reportMatch(self)
                            self.consume()
                        self.state = 173
                        self.expr(6)
                        pass

                    elif la_ == 4:
                        localctx = PseudocodeParser.OrExprContext(self, PseudocodeParser.ExprContext(self, _parentctx, _parentState))
                        self.pushNewRecursionContext(localctx, _startState, self.RULE_expr)
                        self.state = 174
                        if not self.precpred(self._ctx, 3):
                            from antlr4.error.Errors import FailedPredicateException
                            raise FailedPredicateException(self, "self.precpred(self._ctx, 3)")
                        self.state = 175
                        self.match(PseudocodeParser.T__28)
                        self.state = 176
                        self.expr(4)
                        pass

                    elif la_ == 5:
                        localctx = PseudocodeParser.AndExprContext(self, PseudocodeParser.ExprContext(self, _parentctx, _parentState))
                        self.pushNewRecursionContext(localctx, _startState, self.RULE_expr)
                        self.state = 177
                        if not self.precpred(self._ctx, 2):
                            from antlr4.error.Errors import FailedPredicateException
                            raise FailedPredicateException(self, "self.precpred(self._ctx, 2)")
                        self.state = 178
                        self.match(PseudocodeParser.T__29)
                        self.state = 179
                        self.expr(3)
                        pass

             
                self.state = 184
                self._errHandler.sync(self)
                _alt = self._interp.adaptivePredict(self._input,14,self._ctx)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.unrollRecursionContexts(_parentctx)
        return localctx


    class AtomContext(ParserRuleContext):
        __slots__ = 'parser'

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NUMBER(self):
            return self.getToken(PseudocodeParser.NUMBER, 0)

        def STRING(self):
            return self.getToken(PseudocodeParser.STRING, 0)

        def NAME(self):
            return self.getToken(PseudocodeParser.NAME, 0)

        def expr(self):
            return self.getTypedRuleContext(PseudocodeParser.ExprContext,0)


        def getRuleIndex(self):
            return PseudocodeParser.RULE_atom

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterAtom" ):
                listener.enterAtom(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitAtom" ):
                listener.exitAtom(self)

        def accept(self, visitor:ParseTreeVisitor):
            if hasattr( visitor, "visitAtom" ):
                return visitor.visitAtom(self)
            else:
                return visitor.visitChildren(self)




    def atom(self):

        localctx = PseudocodeParser.AtomContext(self, self._ctx, self.state)
        self.enterRule(localctx, 30, self.RULE_atom)
        try:
            self.state = 192
            self._errHandler.sync(self)
            token = self._input.LA(1)
            if token in [35]:
                self.enterOuterAlt(localctx, 1)
                self.state = 185
                self.match(PseudocodeParser.NUMBER)
                pass
            elif token in [36]:
                self.enterOuterAlt(localctx, 2)
                self.state = 186
                self.match(PseudocodeParser.STRING)
                pass
            elif token in [34]:
                self.enterOuterAlt(localctx, 3)
                self.state = 187
                self.match(PseudocodeParser.NAME)
                pass
            elif token in [32]:
                self.enterOuterAlt(localctx, 4)
                self.state = 188
                self.match(PseudocodeParser.T__31)
                self.state = 189
                self.expr(0)
                self.state = 190
                self.match(PseudocodeParser.T__32)
                pass
            else:
                raise NoViableAltException(self)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx



    def sempred(self, localctx:RuleContext, ruleIndex:int, predIndex:int):
        if self._predicates == None:
            self._predicates = dict()
        self._predicates[14] = self.expr_sempred
        pred = self._predicates.get(ruleIndex, None)
        if pred is None:
            raise Exception("No predicate with index:" + str(ruleIndex))
        else:
            return pred(localctx, predIndex)

    def expr_sempred(self, localctx:ExprContext, predIndex:int):
            if predIndex == 0:
                return self.precpred(self._ctx, 9)
         

            if predIndex == 1:
                return self.precpred(self._ctx, 7)
         

            if predIndex == 2:
                return self.precpred(self._ctx, 5)
         

            if predIndex == 3:
                return self.precpred(self._ctx, 3)
         

            if predIndex == 4:
                return self.precpred(self._ctx, 2)
         




