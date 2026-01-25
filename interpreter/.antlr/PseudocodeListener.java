// Generated from /home/bujor/Uni/Licenta/pseudo/interpreter/Pseudocode.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link PseudocodeParser}.
 */
public interface PseudocodeListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#program}.
	 * @param ctx the parse tree
	 */
	void enterProgram(PseudocodeParser.ProgramContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#program}.
	 * @param ctx the parse tree
	 */
	void exitProgram(PseudocodeParser.ProgramContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#stmt}.
	 * @param ctx the parse tree
	 */
	void enterStmt(PseudocodeParser.StmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#stmt}.
	 * @param ctx the parse tree
	 */
	void exitStmt(PseudocodeParser.StmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#assign}.
	 * @param ctx the parse tree
	 */
	void enterAssign(PseudocodeParser.AssignContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#assign}.
	 * @param ctx the parse tree
	 */
	void exitAssign(PseudocodeParser.AssignContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#swapStmt}.
	 * @param ctx the parse tree
	 */
	void enterSwapStmt(PseudocodeParser.SwapStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#swapStmt}.
	 * @param ctx the parse tree
	 */
	void exitSwapStmt(PseudocodeParser.SwapStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#multiStmt}.
	 * @param ctx the parse tree
	 */
	void enterMultiStmt(PseudocodeParser.MultiStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#multiStmt}.
	 * @param ctx the parse tree
	 */
	void exitMultiStmt(PseudocodeParser.MultiStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#readStmt}.
	 * @param ctx the parse tree
	 */
	void enterReadStmt(PseudocodeParser.ReadStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#readStmt}.
	 * @param ctx the parse tree
	 */
	void exitReadStmt(PseudocodeParser.ReadStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#writeStmt}.
	 * @param ctx the parse tree
	 */
	void enterWriteStmt(PseudocodeParser.WriteStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#writeStmt}.
	 * @param ctx the parse tree
	 */
	void exitWriteStmt(PseudocodeParser.WriteStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#nameList}.
	 * @param ctx the parse tree
	 */
	void enterNameList(PseudocodeParser.NameListContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#nameList}.
	 * @param ctx the parse tree
	 */
	void exitNameList(PseudocodeParser.NameListContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#exprList}.
	 * @param ctx the parse tree
	 */
	void enterExprList(PseudocodeParser.ExprListContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#exprList}.
	 * @param ctx the parse tree
	 */
	void exitExprList(PseudocodeParser.ExprListContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#ifStmt}.
	 * @param ctx the parse tree
	 */
	void enterIfStmt(PseudocodeParser.IfStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#ifStmt}.
	 * @param ctx the parse tree
	 */
	void exitIfStmt(PseudocodeParser.IfStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#thenBlock}.
	 * @param ctx the parse tree
	 */
	void enterThenBlock(PseudocodeParser.ThenBlockContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#thenBlock}.
	 * @param ctx the parse tree
	 */
	void exitThenBlock(PseudocodeParser.ThenBlockContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#elseBlock}.
	 * @param ctx the parse tree
	 */
	void enterElseBlock(PseudocodeParser.ElseBlockContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#elseBlock}.
	 * @param ctx the parse tree
	 */
	void exitElseBlock(PseudocodeParser.ElseBlockContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#forStmt}.
	 * @param ctx the parse tree
	 */
	void enterForStmt(PseudocodeParser.ForStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#forStmt}.
	 * @param ctx the parse tree
	 */
	void exitForStmt(PseudocodeParser.ForStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#whileStmt}.
	 * @param ctx the parse tree
	 */
	void enterWhileStmt(PseudocodeParser.WhileStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#whileStmt}.
	 * @param ctx the parse tree
	 */
	void exitWhileStmt(PseudocodeParser.WhileStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#doWhileStmt}.
	 * @param ctx the parse tree
	 */
	void enterDoWhileStmt(PseudocodeParser.DoWhileStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#doWhileStmt}.
	 * @param ctx the parse tree
	 */
	void exitDoWhileStmt(PseudocodeParser.DoWhileStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#repeatStmt}.
	 * @param ctx the parse tree
	 */
	void enterRepeatStmt(PseudocodeParser.RepeatStmtContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#repeatStmt}.
	 * @param ctx the parse tree
	 */
	void exitRepeatStmt(PseudocodeParser.RepeatStmtContext ctx);
	/**
	 * Enter a parse tree produced by the {@code notExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterNotExpr(PseudocodeParser.NotExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code notExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitNotExpr(PseudocodeParser.NotExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code addExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterAddExpr(PseudocodeParser.AddExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code addExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitAddExpr(PseudocodeParser.AddExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code negExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterNegExpr(PseudocodeParser.NegExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code negExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitNegExpr(PseudocodeParser.NegExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code sqrtExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterSqrtExpr(PseudocodeParser.SqrtExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code sqrtExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitSqrtExpr(PseudocodeParser.SqrtExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code mulExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterMulExpr(PseudocodeParser.MulExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code mulExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitMulExpr(PseudocodeParser.MulExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code atomExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterAtomExpr(PseudocodeParser.AtomExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code atomExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitAtomExpr(PseudocodeParser.AtomExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code orExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterOrExpr(PseudocodeParser.OrExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code orExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitOrExpr(PseudocodeParser.OrExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code int}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterInt(PseudocodeParser.IntContext ctx);
	/**
	 * Exit a parse tree produced by the {@code int}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitInt(PseudocodeParser.IntContext ctx);
	/**
	 * Enter a parse tree produced by the {@code compareExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterCompareExpr(PseudocodeParser.CompareExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code compareExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitCompareExpr(PseudocodeParser.CompareExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code andExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterAndExpr(PseudocodeParser.AndExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code andExpr}
	 * labeled alternative in {@link PseudocodeParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitAndExpr(PseudocodeParser.AndExprContext ctx);
	/**
	 * Enter a parse tree produced by {@link PseudocodeParser#atom}.
	 * @param ctx the parse tree
	 */
	void enterAtom(PseudocodeParser.AtomContext ctx);
	/**
	 * Exit a parse tree produced by {@link PseudocodeParser#atom}.
	 * @param ctx the parse tree
	 */
	void exitAtom(PseudocodeParser.AtomContext ctx);
}