# Animal-Guesser-Self-Learning-Decision-Tree-
An interactive Animal Guesser game developed in C++ that asks users a series of questions to guess the animal they are thinking of. Built using logical conditions and decision-making concepts, this project demonstrates basic programming skills, user interaction, and problem-solving in C++.
# Animal Guesser

A simple, interactive **self-learning animal guessing game** written in
**C++17**. Think of an animal, answer a series of yes/no questions, and
the program will try to guess it. If it guesses incorrectly, you can
teach it a new animal and a question that distinguishes it from its
incorrect guess.

The program stores its knowledge in a text file, so the animals it
learns are available the next time you run it.

## Features

-   **Self-learning decision tree:** Learns new animals when it makes an
    incorrect guess.
-   **Yes/no questions:** Uses a binary decision tree to narrow down the
    animal.
-   **Persistent knowledge:** Saves and loads the knowledge tree using
    `animals.txt`.
-   **Knowledge tree viewer:** Displays the questions and animal guesses
    in the tree.
-   **Known animal list:** Shows all animals currently stored.
-   **Game statistics:** Displays animals known, tree depth, games
    played, correct guesses, learned animals, and accuracy.
-   **Reset option:** Restores the built-in starter knowledge.

## Requirements

-   A C++ compiler that supports **C++17** (for example, GCC or Clang).
-   A terminal or command prompt.

No third-party libraries are required. The program uses the C++ standard
library.

## Project Structure

``` text
.
├── animal_guesser.cpp   # Main C++ source code
├── animals.txt          # Saved knowledge (created automatically)
└── README.md            # Project documentation
```

`animals.txt` is created when the program saves its knowledge. You do
not need to create it manually.

## Compile

Open a terminal in the project directory and run:

``` bash
g++ -std=c++17 -O2 -Wall -Wextra -o animal_guesser animal_guesser.cpp
```

## Run

On Linux or macOS:

``` bash
./animal_guesser
```

On Windows (if using MinGW):

``` bash
animal_guesser.exe
```

## How to Play

1.  Start the program and choose **1) Play**.
2.  Think of an animal.
3.  Answer the questions with `y` or `n`.
4.  The program makes an animal guess.
5.  If the guess is correct, answer `y`.
6.  If the guess is wrong, enter the animal you were thinking of.
7.  Provide a yes/no question that distinguishes your animal from the
    program's incorrect guess.
8.  Tell the program whether the answer to your new question is yes for
    your animal.
9.  The program adds the new information to its decision tree.

Choose **0) Save & quit** to save the knowledge and exit.

## Menu Options

  Option   Action
  -------- -----------------------------------------
  `1`      Play the animal guessing game
  `2`      Show the knowledge decision tree
  `3`      List known animals
  `4`      Display game statistics
  `5`      Save knowledge to `animals.txt`
  `6`      Load knowledge from `animals.txt`
  `7`      Reset to the built-in starter knowledge
  `0`      Save knowledge and quit

## How It Works

The game uses a **binary decision tree**:

-   Each internal node contains a yes/no question.
-   Each leaf node contains an animal name.
-   A `yes` answer follows the left/yes branch.
-   A `no` answer follows the right/no branch.
-   When the program guesses incorrectly, it replaces the incorrect
    animal leaf with a new question and two animal leaves.

The built-in starter tree contains 36 animals. As you teach the program
more animals, the decision tree grows.

## Saved Knowledge Format

The knowledge tree is stored in `animals.txt` in a pre-order text
format:

-   `Q|question` represents a question node.
-   `A|animal` represents an animal leaf.

The yes subtree is stored before the no subtree. The program reads this
file when it starts and saves the current tree when you quit.

## Notes

-   Run the program from the project directory so it can find or create
    `animals.txt` there.
-   Keep `animals.txt` if you want to preserve the animals the program
    has learned.
-   Choosing the reset option replaces the current in-memory tree with
    the built-in starter knowledge. The reset is saved when you next
    save or quit.
-   This is a terminal-based C++ project; it does not require a
    graphical interface.

## License

No license is specified yet. Add a license file if you want others to
use, modify, and distribute this project under defined terms.
