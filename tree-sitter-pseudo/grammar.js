/**
 * @file Romanian BAC Pseudocode grammar for tree-sitter
 * @author Bujor Mihai Alexandru <bujormihaialexandru@gmail.com>
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

const PREC = {
  OR: 1,      // SAU/sau
  AND: 2,     // SI/si
  NOT: 3,     // NOT/not
  COMPARE: 4, // = != < <= > >=
  ADD: 5,     // + -
  MUL: 6,     // * / %
  UNARY: 7,   // - (negation), sqrt, floor
};

module.exports = grammar({
  name: "pseudo",

  extras: $ => [
    /\s+/,
    $.comment,
  ],

  word: $ => $.identifier,

  rules: {
    program: $ => repeat($.stmt),

    stmt: $ => choice(
      $.assign,
      $.swap,
      $.read,
      $.write,
      $.if,
      $.for,
      $.while,
      $.do_while,
      $.repeat,
      $.multi_stmt,
    ),

    // x <- expr
    assign: $ => seq(
      field('name', $.identifier),
      '<-',
      field('value', $.expr),
    ),

    // x <-> y or x <--> y
    swap: $ => seq(
      field('left', $.identifier),
      choice('<->', '<-->'),
      field('right', $.identifier),
    ),

    // citeste x, y, z
    read: $ => seq(
      'citeste',
      field('names', $.name_list),
    ),

    // scrie expr1, expr2
    write: $ => seq(
      'scrie',
      field('values', $.expr_list),
    ),

    name_list: $ => seq(
      $.identifier,
      repeat(seq(',', $.identifier)),
    ),

    expr_list: $ => seq(
      $.expr,
      repeat(seq(',', $.expr)),
    ),

    // Multi-statement: x <- 1; y <- 2
    multi_stmt: $ => seq(
      $._simple_stmt,
      repeat1(seq(';', $._simple_stmt)),
    ),

    _simple_stmt: $ => choice(
      $.assign,
      $.swap,
      $.read,
      $.write,
    ),

    // daca expr atunci ... [altfel ...] sf
    if: $ => seq(
      'daca',
      field('condition', $.expr),
      'atunci',
      repeat($.stmt),
      optional(seq(
        'altfel',
        repeat($.stmt),
      )),
      'sf',
    ),

    // pentru i <- start, end[, step] executa ... sf
    for: $ => seq(
      'pentru',
      field('var', $.identifier),
      '<-',
      field('start', $.expr),
      ',',
      field('end', $.expr),
      optional(seq(',', field('step', $.expr))),
      'executa',
      repeat($.stmt),
      'sf',
    ),

    // cat timp expr executa ... sf
    while: $ => seq(
      'cat',
      'timp',
      field('condition', $.expr),
      'executa',
      repeat($.stmt),
      'sf',
    ),

    // executa ... cat timp expr
    do_while: $ => prec.right(1, seq(
      'executa',
      repeat($.stmt),
      'cat',
      'timp',
      field('condition', $.expr),
    )),

    // repeta ... pana cand expr
    repeat: $ => prec.right(1, seq(
      'repeta',
      repeat($.stmt),
      'pana',
      'cand',
      field('condition', $.expr),
    )),

    // Expression rules with proper precedence
    expr: $ => choice(
      $._binary_expr,
      $._unary_expr,
      $.floor,
      $.paren,
      $.atom,
    ),

    _binary_expr: $ => choice(
      $.or_expr,
      $.and_expr,
      $.compare_expr,
      $.add_expr,
      $.mul_expr,
    ),

    _unary_expr: $ => choice(
      $.not_expr,
      $.neg_expr,
      $.sqrt_expr,
    ),

    // Binary operators
    or_expr: $ => prec.left(PREC.OR, seq(
      field('left', $.expr),
      field('op', choice('SAU', 'sau')),
      field('right', $.expr),
    )),

    and_expr: $ => prec.left(PREC.AND, seq(
      field('left', $.expr),
      field('op', choice('SI', 'si')),
      field('right', $.expr),
    )),

    compare_expr: $ => prec.left(PREC.COMPARE, seq(
      field('left', $.expr),
      field('op', choice('=', '!=', '<', '<=', '>', '>=')),
      field('right', $.expr),
    )),

    add_expr: $ => prec.left(PREC.ADD, seq(
      field('left', $.expr),
      field('op', choice('+', '-')),
      field('right', $.expr),
    )),

    mul_expr: $ => prec.left(PREC.MUL, seq(
      field('left', $.expr),
      field('op', choice('*', '/', '%')),
      field('right', $.expr),
    )),

    // Unary operators
    not_expr: $ => prec(PREC.NOT, seq(
      field('op', choice('NOT', 'not')),
      field('operand', $.expr),
    )),

    neg_expr: $ => prec(PREC.UNARY, seq(
      '-',
      field('operand', $.atom),
    )),

    sqrt_expr: $ => prec(PREC.UNARY, seq(
      '\u221A', // √ symbol
      field('operand', $.atom),
    )),

    // Floor: [expr]
    floor: $ => prec(PREC.UNARY, seq(
      '[',
      field('operand', $.expr),
      ']',
    )),

    // Parenthesized expression
    paren: $ => seq('(', $.expr, ')'),

    // Atoms: literals and identifiers
    atom: $ => choice(
      $.number,
      $.string,
      $.identifier,
    ),

    number: $ => /[0-9]+(\.[0-9]+)?/,

    string: $ => choice(
      seq('"', /[^"\r\n]*/, '"'),
      seq("'", /[^'\r\n]*/, "'"),
    ),

    identifier: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,

    comment: $ => seq('#', /[^\r\n]*/),
  },
});
