import XCTest
import SwiftTreeSitter
import TreeSitterPseudo

final class TreeSitterPseudoTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_pseudo())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Pseudo grammar")
    }
}
