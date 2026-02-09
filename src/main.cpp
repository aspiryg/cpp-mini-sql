#include "tokenizer.h"
#include "parser.h"
#include "executor.h"
#include "database.h"
#include <iostream>
#include <string>

void printBanner() {
    std::cout << R"(
  __  __ _       _   ____   ___  _     
 |  \/  (_)_ __ (_) / ___| / _ \| |    
 | |\/| | | '_ \| | \___ \| | | | |    
 | |  | | | | | | |  ___) | |_| | |___ 
 |_|  |_|_|_| |_|_| |____/ \__\_\_____|

)" << std::endl;
    std::cout << "Mini SQL Engine v1.0 — A learning project" << std::endl;
    std::cout << "Supports: CREATE TABLE, INSERT INTO, SELECT ... WHERE" << std::endl;
    std::cout << "Type 'exit' or 'quit' to leave. Type 'help' for examples.\n" << std::endl;
}

void printHelp() {
    std::cout << "\n--- Supported SQL Commands ---\n\n";
    std::cout << "  CREATE TABLE users (id INT, name TEXT, age INT);\n";
    std::cout << "  INSERT INTO users VALUES (1, 'Alice', 30);\n";
    std::cout << "  INSERT INTO users VALUES (2, 'Bob', 25);\n";
    std::cout << "  SELECT * FROM users;\n";
    std::cout << "  SELECT name, age FROM users WHERE age > 20;\n";
    std::cout << "  SELECT * FROM users WHERE age >= 25 AND name = 'Alice';\n\n";
    std::cout << "  Column types: INT, TEXT\n";
    std::cout << "  Operators:    =  !=  <  >  <=  >=\n";
    std::cout << "  Logic:        AND  OR\n\n";
}

int main() {
    printBanner();

    Database db;
    std::string line;

    while (true) {
        std::cout << "sql> ";
        if (!std::getline(std::cin, line)) break;

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        // Check for special commands
        if (line == "exit" || line == "quit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        if (line == "help") {
            printHelp();
            continue;
        }

        try {
            // 1. Tokenize
            Tokenizer tokenizer(line);
            auto tokens = tokenizer.tokenize();

            // 2. Parse
            Parser parser(tokens);
            Statement stmt = parser.parse();

            // 3. Execute
            Executor executor(db);
            QueryResult result = executor.execute(stmt);

            // 4. Display
            std::cout << Executor::formatResult(result) << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }

    return 0;
}
