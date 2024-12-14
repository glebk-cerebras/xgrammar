#include <xgrammar/xgrammar.h>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <set>

int main() {
    // Load list of tokens from file to vector of strings.
    std::cout << "Loading tokens..." << std::endl;
    std::vector<std::string> tokens;
    std::ifstream tokens_file("/net/glebk-dev/srv/nfs/glebk-data/ws/monolith/src/ws/stack/ws-run/Inference/plugins/constrained_output/cerebras/encoded_vocab.txt");
    std::string token;
    while (std::getline(tokens_file, token)) {
        tokens.push_back(token);
    }
    int vocab_size = tokens.size();
    std::cout << "Loaded " << vocab_size << " tokens" << std::endl;
    
    // Initialize compiler.
    std::cout << "Initializing compiler..." << std::endl;
    auto compiler = std::make_unique<xgrammar::GrammarCompiler>(
        xgrammar::TokenizerInfo(
            tokens,
            xgrammar::VocabType::BYTE_FALLBACK,
            vocab_size,
            std::vector<int32_t>{128009},
            false
        ), 1 /* num_threads */, true /* enable cache */
    );

    // Read grammar from file as string.
    std::cout << "Reading grammar..." << std::endl;
    std::ifstream grammar_file("/cb/home/glebk/ws/monolith/src/inference/grammar/json.gbnf");
    std::string grammar((std::istreambuf_iterator<char>(grammar_file)), std::istreambuf_iterator<char>());

    // Compile grammar.
    std::cout << "Compiling grammar..." << std::endl;
    auto compile_grammar = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        auto tmp = compiler->CompileGrammar(
            xgrammar::Grammar::FromEBNF(grammar, "root")
        );
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Compiled completed in: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
        return tmp;
    };
    
    auto compiled_grammar = compile_grammar();
    // Checking if the cache works.
    compiled_grammar = compile_grammar();


    std::cout << "Initialize matcher..." << std::endl;
    auto matcher = xgrammar::GrammarMatcher(std::move(compiled_grammar));

    // std::cout << "Accepting token..." << std::endl;
    // matcher.AcceptToken(90);

    std::cout << "Filling token bitmask..." << std::endl;
    auto buffer_size = xgrammar::GetBitmaskSize(vocab_size);
    std::vector<int32_t> result(buffer_size);

    std::vector<int64_t> shape = {static_cast<int64_t>(result.size())};
    matcher.FillNextTokenBitmask(reinterpret_cast<intptr_t>(result.data()), shape);

    std::cout << "Checking token bitmask..." << std::endl;

    std::cout << "Valid tokens:" << std::endl;
    for (int index = 0; index < vocab_size; index++) {
        if ((result[index / 32] >> (index % 32)) & 1) {
            std::cout << index << " : " << tokens[index] << std::endl;
        }
    }

    // xgrammar::DynamicBitset bitset(vocab_size, reinterpret_cast<uint32_t*>(result.data()));

    // std::set<int> rejected_tokens;
    // for (int i = bitset.FindFirstZero(); i != -1; i = bitset.FindNextZero(i)) {
    //     rejected_tokens.insert(i);
    // }

    // std::cout << "Rejected tokens: " << rejected_tokens.size() << std::endl;
    // std::cout << "Valid tokens: " << vocab_size - rejected_tokens.size() << std::endl;
    // for (int token = 0; token < vocab_size; token++) {
    //     if (rejected_tokens.find(token) == rejected_tokens.end()) {
    //         std::cout << token << " : " << tokens[token] << std::endl;
    //     }
    // }
    // for (int token = 0; token < vocab_size; token++) {
    //     if (result[token] != 0) {
    //         std::cout << token << std::endl;
    //     }
    // }
    std::cout << "done!" << std::endl;
}
