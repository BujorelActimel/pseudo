import sys
import re
import argparse

class PseudocodeLinter:
    def __init__(self, tab = 4*' '):
        self.char_replacements = {
            '≤': '<=',
            '': '<=',
            '≠': '!=', 
            '≥': '>=',
            '': '<-',
            '': '<-',
            '': '<->',
            '→': '->',
            '': '<-',
            '←': '<-',
            '■': 'sf',
            '│ ': tab,
            '│': tab,
            '’': "'",
            '‘': "'",
        }
        
        self.box_chars = ['┌', '└']

    def clean_line(self, line):
        line = line.strip()
        
        if not line:
            return ""
            
        if any(char in line for char in self.box_chars):
            line = self._handle_box_structure(line)
        
        for old_char, new_char in self.char_replacements.items():
            line = line.replace(old_char, new_char)
                
        line = self._handle_keywords(line)
        
        return line
    
    def _handle_box_structure(self, line):
        for char in self.box_chars:
            line = line.replace(char, '')
        
        line = line.strip()
        
        return line
    
    def _handle_keywords(self, line):
        keywords = {
            'citeşte': 'citeste',
            'citește': 'citeste',
            'cât timp': 'cat timp', 
            'până când': 'pana cand',
            'dacă': 'daca',
            'atunci': 'atunci',
            'altfel': 'altfel',
            'execută': 'executa',
            'repetă': 'repeta',
            'și': 'si',
            'şi': 'si',
        }
        
        for old_keyword, new_keyword in keywords.items():
            line = line.replace(old_keyword, new_keyword)
                
        return line
    
    def process_text(self, text):
        lines = text.split('\n')
        cleaned_lines = []
        
        for line in lines:
            cleaned = self.clean_line(line)
            cleaned_lines.append(cleaned)
        
        return '\n'.join(cleaned_lines)
    
    def process_file(self, input_file, output_file=None):
        try:
            with open(input_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            cleaned_content = self.process_text(content)
            
            if output_file:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(cleaned_content)
                print(f"Cleaned pseudocode written to {output_file}")
            else:
                print(cleaned_content)
                
        except FileNotFoundError:
            print(f"Error: File {input_file} not found")
        except Exception as e:
            print(f"Error processing file: {e}")

def main():
    parser = argparse.ArgumentParser(description='Clean pseudocode from PDF copy-paste format')
    parser.add_argument('input_file', nargs='?', help='Input pseudocode file')
    parser.add_argument('-o', '--output', help='Output file (default: print to stdout)')
    parser.add_argument('--interactive', '-i', action='store_true', 
                       help='Interactive mode - paste text directly')
    
    args = parser.parse_args()
    
    linter = PseudocodeLinter()
    
    if args.interactive:
        print("Enter pseudocode (Ctrl+D or Ctrl+Z to finish):")
        try:
            text = sys.stdin.read()
            cleaned = linter.process_text(text)
            print("\nCleaned pseudocode:")
            print(cleaned)
        except KeyboardInterrupt:
            print("\nAborted.")
    else:
        linter.process_file(args.input_file, args.output)

if __name__ == '__main__':
    main()
