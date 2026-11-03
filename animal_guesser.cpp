// ============================================================================
//  Animal Guesser - a self-learning decision tree (C++17, standard library only)
//
//  Build:  g++ -std=c++17 -O2 -Wall -Wextra -o animal_guesser animal_guesser.cpp
//  Run:    ./animal_guesser
//
//  How it works
//  ------------
//  * The knowledge base is a binary decision tree.
//      - Internal node = a yes/no question   (yes -> left, no -> right)
//      - Leaf node     = an animal guess
//  * The program walks the tree by asking questions, then guesses at a leaf.
//  * If the guess is wrong, it learns: it asks for the correct animal and a
//    question that separates it from the wrong guess, then replaces that leaf
//    with a new question node. The tree grows smarter every game.
//  * The tree is saved to / loaded from a text file, so learning persists.
// ============================================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using std::string;

// ----------------------------------------------------------------------------
//  Small string / input helpers
// ----------------------------------------------------------------------------
namespace util {

string trim(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

string toLower(string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Reads one line from stdin. Returns false on EOF.
bool readLine(const string& prompt, string& out) {
    std::cout << prompt << std::flush;
    if (!std::getline(std::cin, out)) return false;
    out = trim(out);
    return true;
}

// Keeps asking until the user types something non-empty. Throws on EOF.
string readNonEmpty(const string& prompt) {
    string line;
    while (true) {
        if (!readLine(prompt, line)) throw std::runtime_error("input closed");
        if (!line.empty()) return line;
        std::cout << "  (please type something)\n";
    }
}

// y/yes/n/no (case-insensitive). Throws on EOF.
bool askYesNo(const string& prompt) {
    while (true) {
        string line;
        if (!readLine(prompt + " (y/n): ", line)) throw std::runtime_error("input closed");
        line = toLower(line);
        if (line == "y" || line == "yes") return true;
        if (line == "n" || line == "no") return false;
        std::cout << "  Please answer y or n.\n";
    }
}

// "tiger" -> "a tiger", "elephant" -> "an elephant"
string withArticle(const string& name) {
    string lower = toLower(name);
    if (lower.rfind("a ", 0) == 0 || lower.rfind("an ", 0) == 0 || lower.rfind("the ", 0) == 0)
        return name;
    if (!lower.empty() && string("aeiou").find(lower[0]) != string::npos) return "an " + name;
    return "a " + name;
}

// Strip leading article so "a Dog", "dog" and "DOG" compare equal.
string normalizeAnimal(const string& name) {
    string s = toLower(trim(name));
    for (const char* art : {"a ", "an ", "the "}) {
        string a(art);
        if (s.rfind(a, 0) == 0) { s = s.substr(a.size()); break; }
    }
    return s;
}

// Make sure a question reads nicely: capitalised, ends with '?'
string normalizeQuestion(string q) {
    q = trim(q);
    if (q.empty()) return q;
    q[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(q[0])));
    if (q.back() != '?') q += '?';
    return q;
}

}  // namespace util

// ----------------------------------------------------------------------------
//  Decision tree
// ----------------------------------------------------------------------------
struct Node {
    string text;                 // question (internal) or animal name (leaf)
    std::unique_ptr<Node> yes;   // followed when answer is "yes"
    std::unique_ptr<Node> no;    // followed when answer is "no"

    explicit Node(string t) : text(std::move(t)) {}
    bool isLeaf() const { return !yes && !no; }
};

class DecisionTree {
public:
    DecisionTree() { reset(); }

    // Built-in starter knowledge (35 questions, 36 animals), see kStarterData.
    void reset() {
        std::istringstream in(kStarterData);
        root_ = readNode(in);
        if (!root_) {  // should never happen, but stay safe
            root_ = std::make_unique<Node>("Does it live in water?");
            root_->yes = std::make_unique<Node>("fish");
            root_->no = std::make_unique<Node>("dog");
        }
    }

    // --- Playing -----------------------------------------------------------
    // Returns true if the game completed (false if input closed mid-game).
    bool playRound() {
        Node* parent = nullptr;
        bool cameFromYes = false;
        Node* cur = root_.get();

        while (!cur->isLeaf()) {
            bool ans = util::askYesNo(cur->text);
            parent = cur;
            cameFromYes = ans;
            cur = ans ? cur->yes.get() : cur->no.get();
        }

        ++stats_.played;
        bool correct = util::askYesNo("Is it " + util::withArticle(cur->text) + "?");
        if (correct) {
            ++stats_.correct;
            std::cout << "  Got it! Better luck stumping me next time.\n";
        } else {
            learn(cur, parent, cameFromYes);
        }
        return true;
    }

    // --- Persistence -------------------------------------------------------
    // Format (pre-order, one node per line):   Q|<question>   or   A|<animal>
    bool save(const string& path) const {
        std::ofstream out(path);
        if (!out) return false;
        writeNode(out, root_.get());
        return static_cast<bool>(out);
    }

    bool load(const string& path) {
        std::ifstream in(path);
        if (!in) return false;
        std::unique_ptr<Node> loaded = readNode(in);
        if (!loaded) return false;
        root_ = std::move(loaded);
        return true;
    }

    // --- Introspection -----------------------------------------------------
    void printTree() const {
        std::cout << "\nKnowledge tree (Y = yes branch, N = no branch):\n";
        printNode(root_.get(), "", "");
        std::cout << '\n';
    }

    std::vector<string> animals() const {
        std::vector<string> out;
        collectAnimals(root_.get(), out);
        std::sort(out.begin(), out.end());
        return out;
    }

    int depth() const { return depthOf(root_.get()); }

    void printStats() const {
        auto list = animals();
        std::cout << "\n--- Statistics ---\n"
                  << "  Animals known : " << list.size() << '\n'
                  << "  Tree depth    : " << depth() << '\n'
                  << "  Games played  : " << stats_.played << '\n'
                  << "  Correct       : " << stats_.correct << '\n'
                  << "  Learned       : " << stats_.learned << '\n';
        if (stats_.played > 0)
            std::cout << "  Accuracy      : "
                      << (100 * stats_.correct / stats_.played) << "%\n";
        std::cout << '\n';
    }

private:
    // Starter knowledge in the same pre-order format used for saving
    // (indentation is optional and only here for readability).
    // Each Q is followed by its YES subtree, then its NO subtree.
    static constexpr const char* kStarterData = R"DATA(
Q|Is it a mammal?
 Q|Does it live mainly in water?
  Q|Is it bigger than a bus?
   A|whale
   Q|Does it bark and have whiskers?
    A|seal
    A|dolphin
  Q|Can it fly?
   A|bat
   Q|Is it a common household pet?
    Q|Does it meow?
     A|cat
     Q|Does it bark?
      A|dog
      A|rabbit
    Q|Does it have a very long neck?
     A|giraffe
     Q|Does it have a trunk?
      A|elephant
      Q|Is it a big cat with spots or stripes?
       Q|Does it have stripes?
        A|tiger
        A|leopard
       Q|Does it carry its young in a pouch?
        A|kangaroo
        Q|Does it have a mane?
         A|lion
         Q|Does it live in trees and have a tail?
          A|monkey
          Q|Is it a farm animal that gives milk?
           A|cow
           A|bear
 Q|Can it fly?
  Q|Is it a bird?
   Q|Does it hunt at night?
    A|owl
    Q|Does it have a hooked beak and hunt?
     A|eagle
     Q|Can it mimic human speech?
      A|parrot
      A|sparrow
   Q|Does it make honey?
    A|bee
    Q|Does it have colorful wings?
     A|butterfly
     A|mosquito
  Q|Does it live in water?
   Q|Does it have fins?
    Q|Does it have sharp teeth and a big fin on its back?
     A|shark
     A|goldfish
    Q|Does it have eight arms?
     A|octopus
     Q|Does it have a shell?
      A|turtle
      A|frog
   Q|Does it slither without legs?
    A|snake
    Q|Does it have six legs?
     A|ant
     Q|Does it have eight legs?
      A|spider
      Q|Is it a bird that cannot fly?
       Q|Does it live in cold places?
        A|penguin
        A|ostrich
       Q|Does it have scales?
        Q|Is it large and lives near rivers?
         A|crocodile
         A|lizard
        A|snail
)DATA";

    struct Stats { int played = 0, correct = 0, learned = 0; };

    std::unique_ptr<Node> root_;
    Stats stats_;

    // Replace a wrong leaf with a question node that separates the two animals.
    void learn(Node* wrongLeaf, Node* parent, bool cameFromYes) {
        string oldAnimal = wrongLeaf->text;

        string newAnimal = util::readNonEmpty("  I give up! What animal were you thinking of? ");

        if (util::normalizeAnimal(newAnimal) == util::normalizeAnimal(oldAnimal)) {
            std::cout << "  Hmm, that's what I guessed - maybe you answered a question wrongly.\n";
            return;
        }
        if (knowsAnimal(newAnimal)) {
            std::cout << "  I already know " << util::withArticle(newAnimal)
                      << ", so one of your earlier answers must have differed from mine.\n"
                         "  (Use 'Show tree' from the menu to inspect what I know.)\n";
            return;
        }

        std::cout << "  Please give me a yes/no question that tells "
                  << util::withArticle(newAnimal) << " apart from "
                  << util::withArticle(oldAnimal) << ".\n";
        string question = util::normalizeQuestion(util::readNonEmpty("  Question: "));

        bool yesForNew = util::askYesNo("  For " + util::withArticle(newAnimal) +
                                        ", what is the answer to that question?");

        // Build the replacement subtree.
        auto questionNode = std::make_unique<Node>(question);
        auto newLeaf = std::make_unique<Node>(util::toLower(util::trim(newAnimal)));
        auto oldLeaf = std::make_unique<Node>(oldAnimal);
        if (yesForNew) {
            questionNode->yes = std::move(newLeaf);
            questionNode->no = std::move(oldLeaf);
        } else {
            questionNode->yes = std::move(oldLeaf);
            questionNode->no = std::move(newLeaf);
        }

        // Splice it in where the wrong leaf used to be.
        if (parent == nullptr)
            root_ = std::move(questionNode);
        else if (cameFromYes)
            parent->yes = std::move(questionNode);
        else
            parent->no = std::move(questionNode);

        ++stats_.learned;
        std::cout << "  Thanks! I've learned " << util::withArticle(newAnimal) << ".\n";
    }

    bool knowsAnimal(const string& name) const {
        string target = util::normalizeAnimal(name);
        for (const auto& a : animals())
            if (util::normalizeAnimal(a) == target) return true;
        return false;
    }

    // --- Recursive helpers -------------------------------------------------
    static void writeNode(std::ostream& out, const Node* n) {
        if (n->isLeaf()) {
            out << "A|" << n->text << '\n';
        } else {
            out << "Q|" << n->text << '\n';
            writeNode(out, n->yes.get());
            writeNode(out, n->no.get());
        }
    }

    // Returns nullptr on malformed input.
    static std::unique_ptr<Node> readNode(std::istream& in) {
        string line;
        do {  // skip blank lines
            if (!std::getline(in, line)) return nullptr;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            line.erase(0, line.find_first_not_of(" \t"));  // allow indentation
        } while (line.empty());
        if (line.size() < 3 || line[1] != '|') return nullptr;

        char kind = line[0];
        auto node = std::make_unique<Node>(line.substr(2));
        if (kind == 'A') return node;
        if (kind != 'Q') return nullptr;

        node->yes = readNode(in);
        if (!node->yes) return nullptr;
        node->no = readNode(in);
        if (!node->no) return nullptr;
        return node;
    }

    static void printNode(const Node* n, const string& prefix, const string& label) {
        std::cout << prefix << label << (n->isLeaf() ? "* " + n->text : n->text) << '\n';
        if (n->isLeaf()) return;
        // Indent grows by 4 per level.
        printNode(n->yes.get(), prefix + "    ", "Y: ");
        printNode(n->no.get(), prefix + "    ", "N: ");
    }

    static void collectAnimals(const Node* n, std::vector<string>& out) {
        if (n->isLeaf()) { out.push_back(n->text); return; }
        collectAnimals(n->yes.get(), out);
        collectAnimals(n->no.get(), out);
    }

    static int depthOf(const Node* n) {
        if (n->isLeaf()) return 0;
        return 1 + std::max(depthOf(n->yes.get()), depthOf(n->no.get()));
    }
};

// ----------------------------------------------------------------------------
//  Menu / main
// ----------------------------------------------------------------------------
static const char* kDefaultFile = "animals.txt";

static void printBanner() {
    std::cout << "=====================================\n"
                 "   ANIMAL GUESSER  (self-learning)   \n"
                 "=====================================\n"
                 "Think of an animal and I'll try to guess it.\n"
                 "If I'm wrong, teach me - I remember for next time!\n\n";
}

static void printMenu() {
    std::cout << "Menu:\n"
                 "  1) Play\n"
                 "  2) Show knowledge tree\n"
                 "  3) List known animals\n"
                 "  4) Statistics\n"
                 "  5) Save knowledge\n"
                 "  6) Load knowledge\n"
                 "  7) Reset to starter knowledge\n"
                 "  0) Save & quit\n";
}

int main() {
    DecisionTree tree;
    printBanner();

    if (tree.load(kDefaultFile))
        std::cout << "(Loaded saved knowledge from '" << kDefaultFile << "'.)\n\n";
    else
        std::cout << "(No saved knowledge found - starting fresh.)\n\n";

    try {
        while (true) {
            printMenu();
            string choice;
            if (!util::readLine("> ", choice)) break;  // EOF

            if (choice == "1") {
                do {
                    std::cout << "\nThink of an animal...\n";
                    tree.playRound();
                } while (util::askYesNo("\nPlay again?"));
                std::cout << '\n';
            } else if (choice == "2") {
                tree.printTree();
            } else if (choice == "3") {
                auto list = tree.animals();
                std::cout << "\nI know " << list.size() << " animals:\n";
                for (const auto& a : list) std::cout << "  - " << a << '\n';
                std::cout << '\n';
            } else if (choice == "4") {
                tree.printStats();
            } else if (choice == "5") {
                std::cout << (tree.save(kDefaultFile) ? "Saved to " : "Could not write ")
                          << "'" << kDefaultFile << "'.\n\n";
            } else if (choice == "6") {
                std::cout << (tree.load(kDefaultFile) ? "Loaded from " : "Could not read valid data from ")
                          << "'" << kDefaultFile << "'.\n\n";
            } else if (choice == "7") {
                if (util::askYesNo("Forget everything I've learned?")) {
                    tree.reset();
                    std::cout << "Knowledge reset.\n\n";
                }
            } else if (choice == "0") {
                break;
            } else {
                std::cout << "Unknown option.\n\n";
            }
        }
    } catch (const std::exception&) {
        std::cout << "\nInput closed.\n";
    }

    if (tree.save(kDefaultFile))
        std::cout << "Knowledge saved to '" << kDefaultFile << "'. Goodbye!\n";
    else
        std::cout << "Warning: could not save knowledge. Goodbye!\n";
    return 0;
}
