Keywords: C++, Automata theory, Deterministic/Non-Deterministic Finite Automation, 

# 📝 Task Objective:

You are required to implement two algorithms that find the minimal Deterministic Finite Automaton (DFA) that accepts either the intersection or the union of languages represented by two given Non-Deterministic Finite Automata (NFA). Specifically, you must implement two functions in C++ with the following signatures:
- DFA unify(const NFA& nfa1, const NFA& nfa2);
- DFA intersect(const NFA& nfa1, const NFA& nfa2);

Both functions should return the minimal DFA that represents the union or intersection of the input languages.

## 📥 Input and Output Structures:

### NFA Structure:
Represents a Non-Deterministic Finite Automaton, which might be deterministic for certain transition functions.
Contains sets of states, alphabet symbols, initial and final states, and a transition function.

### DFA Structure:
Represents a Deterministic Finite Automaton.
Contains similar attributes as NFA but adheres strictly to deterministic transitions.

## 📌 Function Requirements:

- unify(const NFA& nfa1, const NFA& nfa2) - Returns the minimal DFA that accepts the union of the languages accepted by nfa1 and nfa2.
- intersect(const NFA& nfa1, const NFA& nfa2) - Returns the minimal DFA that accepts the intersection of the languages accepted by nfa1 and nfa2.

## ✅ Input Guarantees:

The NFAs provided as inputs to unify and intersect will be valid and will adhere to the following constraints:
The sets of states (NFA::m_States) and alphabet symbols (NFA::m_Alphabet) will be non-empty.
The initial (NFA::m_InitialState) and final states (NFA::m_FinalStates) will be elements of the set of states (NFA::m_States).
If a transition is not defined for a state q and symbol a, the key (q, a) will not exist in NFA::m_Transitions.
The transition table (NFA::m_Transitions) will only contain states and symbols specified in the state set and alphabet set.

## 🎯 Output Requirements:
The resulting DFA must satisfy the following conditions:
The output DFA should be minimal, satisfying the minimal DFA properties discussed in the lectures.
If the DFA represents an empty language, it should be a single-state automaton with the unchanged alphabet, an empty set of transitions, and an empty set of final states.
The DFA should accept the correct language, even if the output differs in state naming from the reference solution.
If the input NFAs have different alphabets, the resulting DFA should operate over the union of these alphabets.
🛠️ Implementation Details:
