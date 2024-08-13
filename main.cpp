#ifndef __PROGTEST__

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include <vector>

using State = unsigned int;
using Symbol = uint8_t;

struct NFA {
    std::set<State> m_States;
    std::set<Symbol> m_Alphabet;
    std::map<std::pair<State, Symbol>, std::set<State>> m_Transitions;
    State m_InitialState;
    std::set<State> m_FinalStates;
};

struct DFA {
    std::set<State> m_States;
    std::set<Symbol> m_Alphabet;
    std::map<std::pair<State, Symbol>, State> m_Transitions;
    State m_InitialState;
    std::set<State> m_FinalStates;
};

#endif
using namespace std;

DFA fullDFA(DFA &a) {

    DFA newDFA{
            {a.m_States},
            {a.m_Alphabet},
            {a.m_Transitions},
            a.m_InitialState,
            {a.m_FinalStates}
    };

    bool isFull = true;

    for (const auto &state: a.m_States) {
        for (const auto &letter: a.m_Alphabet) {
            if (a.m_Transitions.find(make_pair(state, letter)) == a.m_Transitions.end()) {
                newDFA.m_Transitions[make_pair(state, letter)] = 1024;
                if (isFull) {
                    newDFA.m_States.insert(1024);
                    for (const auto &alpha: a.m_Alphabet)
                        newDFA.m_Transitions[make_pair(1024, alpha)] = 1024;
                    isFull = false;
                }
            }
        }
    }

    return newDFA;

}

DFA determize(const NFA &a) {
    DFA newAutomata{{}, {a.m_Alphabet}, {}, a.m_InitialState, {}};
    map<set<State>, State> renameStates;
    State counter = 0;

    for (const auto &state: a.m_States)
        renameStates[{state}] = counter++;

    queue<set<State>> q;
    q.push({a.m_InitialState});

    if (a.m_FinalStates.count(a.m_InitialState))
        newAutomata.m_FinalStates.insert(renameStates.at({a.m_InitialState}));

    while (!q.empty()) {
        auto setOfStates = q.front();
        q.pop();

        State state = renameStates[setOfStates];

        if (newAutomata.m_States.count(state))
            continue;

        newAutomata.m_States.insert(state);

        for (const auto &letter: a.m_Alphabet) {
            set<State> newState;
            for (const auto &st: setOfStates) {
                if (a.m_Transitions.find(make_pair(st, letter)) != a.m_Transitions.end())
                    for (const auto &aLetter: a.m_Transitions.at(make_pair(st, letter)))
                        newState.insert(aLetter);

            }

            if (newState.size() == 0)
                continue;

            auto letterState = 0;
            if (renameStates.find(newState) == renameStates.end()) {
                renameStates[newState] = counter;
                letterState = counter;
                counter++;
            } else
                letterState = renameStates[newState];

            newAutomata.m_Transitions[make_pair(state, letter)] = letterState;

            if (newAutomata.m_States.find(letterState) == newAutomata.m_States.end())
                q.push(newState);

            for (const auto &finalState: a.m_FinalStates) {
                if (newState.find(finalState) != newState.end()) {
                    newAutomata.m_FinalStates.insert(letterState);
                    break;
                }
            }
        }
    }

    if (!newAutomata.m_FinalStates.size()) {
        for (const auto &it: a.m_FinalStates)
            if (newAutomata.m_States.count(it))
                newAutomata.m_FinalStates.insert(it);
    }

    return newAutomata;
}

DFA renameAutomat(const DFA &a, DFA &b) {
    unsigned int maxVal = 0;

    for (const auto &it: a.m_Transitions)
        if (it.first.first > maxVal)
            maxVal = it.first.first;

    ++maxVal;

    DFA newAutomata{
            {},
            {b.m_Alphabet},
            {},
            {},
            {}
    };

    map<State, State> renameStates;
    for (const auto &it: b.m_Transitions)
        if (renameStates.find(it.first.first) == renameStates.end()) {
            newAutomata.m_States.insert(maxVal);
            renameStates[it.first.first] = maxVal++;
        }


    for (const auto &it: b.m_Transitions)
        newAutomata.m_Transitions.insert({{
                                                  renameStates[it.first.first],
                                                  it.first.second
                                          }, renameStates[it.second]});

    newAutomata.m_InitialState = renameStates[b.m_InitialState];
    for (const auto &state: b.m_FinalStates)
        newAutomata.m_FinalStates.insert(renameStates[state]);

    return newAutomata;
}

DFA parallelRun(const DFA &a, const DFA &b, bool un) {
    DFA newAutomata{
            {},
            {a.m_Alphabet},
            {},
            {},
            {}
    };

    map<pair<State, Symbol>, State> allStates;

    for (const auto &it: a.m_Transitions)
        allStates[{it.first.first, it.first.second}] = it.second;

    for (const auto &it: b.m_Transitions)
        allStates[{it.first.first, it.first.second}] = it.second;

    set<State> finalStates1;
    set<State> finalStates2;

    for (const auto &state: a.m_FinalStates)
        finalStates1.insert(state);

    for (const auto &state: b.m_FinalStates)
        finalStates2.insert(state);

    map<vector<State>, State> renamedStates;
    unsigned int maxVal = 0;


    queue<vector<State>> q;
    q.push({a.m_InitialState, b.m_InitialState});
    renamedStates[{a.m_InitialState, b.m_InitialState}] = maxVal++;

    set<vector<State>> visited;

    if (un) {
        if (finalStates1.count(a.m_InitialState) || finalStates2.count(b.m_InitialState))
            newAutomata.m_FinalStates.insert(renamedStates[{a.m_InitialState, b.m_InitialState}]);

    } else {
        if (finalStates1.count(a.m_InitialState) && finalStates2.count(b.m_InitialState))
            newAutomata.m_FinalStates.insert(renamedStates[{a.m_InitialState, b.m_InitialState}]);
    }

    while (!q.empty()) {
        auto setOfStates = q.front();
        q.pop();

        if (visited.count(setOfStates))
            continue;

        visited.insert(setOfStates);

        State mainState = renamedStates[setOfStates];

        vector<State> newState;
        for (const auto &letter: a.m_Alphabet) {
            for (const auto &state: setOfStates)
                newState.push_back(allStates.find({state, letter})->second);


            if (renamedStates.find(newState) == renamedStates.end()) {
                renamedStates[newState] = maxVal++;
                q.push(newState);
            }

            newAutomata.m_Transitions[{mainState, letter}] = renamedStates[newState];
            if (un) {
                if (finalStates1.count(newState[0]) || finalStates2.count(newState[1]))
                    newAutomata.m_FinalStates.insert(renamedStates[newState]);

            } else {
                if (finalStates1.count(newState[0]) && finalStates2.count(newState[1]))
                    newAutomata.m_FinalStates.insert(renamedStates[newState]);
            }
            newState.clear();
        }
    }

    newAutomata.m_InitialState = renamedStates[{a.m_InitialState, b.m_InitialState}];
    for (const auto &it: renamedStates)
        newAutomata.m_States.insert(it.second);

    return newAutomata;
}

DFA deleteUselessStates(const DFA &a) {
    DFA newAutomata{
            {},
            {a.m_Alphabet},
            {},
            a.m_InitialState,
            {a.m_FinalStates}
    };
    multimap<State, State> allStatesForReachable;
    multimap<State, State> allStatesForUsefull;

    set<State> reachableStates;
    set<State> usefullStates;
    set<State> allStates;
    for (const auto &it: a.m_Transitions) {
        allStatesForReachable.insert(pair<State, State>(it.first.first, it.second));
        allStatesForUsefull.insert(pair<State, State>(it.second, it.first.first));
        allStates.insert(it.first.first);
    }

    set<State> visited;
    queue<State> q;
    q.push(a.m_InitialState);
    reachableStates.insert(a.m_InitialState);

    while (!q.empty()) {
        auto state = q.front();
        q.pop();

        auto iteratorLower = allStatesForReachable.lower_bound(state);
        for (auto it = iteratorLower; it->first == state; ++it) {
            if (!reachableStates.count(it->second)) {
                q.push(it->second);
                reachableStates.insert(it->second);
            }
        }
    }

    visited.clear();
    for (const auto &it: a.m_FinalStates) {
        q.push(it);
        usefullStates.insert(it);
    }

    while (!q.empty()) {
        auto state = q.front();
        q.pop();

        auto iteratorLower = allStatesForUsefull.lower_bound(state);
        for (auto it = iteratorLower; it->first == state; ++it) {
            if (!usefullStates.count(it->second)) {
                q.push(it->second);
                usefullStates.insert(it->second);
            }
        }
    }

    set<State> intersectionStates;

    if (!usefullStates.size())
        usefullStates = allStates;

    set_difference(allStates.begin(), allStates.end(), usefullStates.begin(), usefullStates.end(),
                   inserter(intersectionStates, intersectionStates.begin()));


    set<State> uselessStates;
    set_difference(allStates.begin(), allStates.end(), intersectionStates.begin(), intersectionStates.end(),
                   inserter(uselessStates, uselessStates.begin()));

    for (const auto &it: a.m_Transitions) {
        if (!intersectionStates.count(it.first.first)) {
            if (!intersectionStates.count(it.second))
                newAutomata.m_Transitions.insert({{it.first.first, it.first.second}, it.second});
            else
                newAutomata.m_Transitions.insert({{it.first.first, it.first.second}, 1024});

            newAutomata.m_States.insert(it.first.first);
        }
    }
    return newAutomata;
}

DFA automataWithoutEquivalentStates(const DFA &a) {
    if (a.m_FinalStates.size() == 0)
        return DFA{
                {a.m_InitialState},
                {a.m_Alphabet},
                {},
                a.m_InitialState,
                {}
        };

    map<State, pair<State, bool> > newStates;
    map<bool, set<State> > boolMap;
    map<State, vector<State> > newTransitions;

    State first_non_final = 0;
    State first_final = *(a.m_FinalStates.begin());
    for (const auto &i: a.m_States)
        if (a.m_FinalStates.find(i) == a.m_FinalStates.end()) {
            first_non_final = i;
            break;
        }

    for (const auto &state: a.m_States) {
        if (a.m_FinalStates.find(state) != a.m_FinalStates.end()) {
            newStates[state] = make_pair(first_final, true);
        } else {
            newStates[state] = make_pair(first_non_final, false);
        }
    }

    boolMap[true].insert(first_final);
    boolMap[false].insert(first_non_final);


    for (const auto &state: a.m_States) {
        for (const auto &alpha: a.m_Alphabet) {
            if (a.m_Transitions.at(make_pair(state, alpha)) == 1024)
                newTransitions[state].push_back(1024);
            else
                newTransitions[state].push_back(newStates.at(a.m_Transitions.at(make_pair(state, alpha))).first);
        }
    }


    while (true) {
        map<State, pair<State, bool> > tmpStates;
        for (const auto &it: a.m_States) {
            bool isFinal = newStates[it].second;
            bool isAdded = false;
            for (const auto &state: boolMap[isFinal]) {
                if (newTransitions[it] == newTransitions[state]) {
                    tmpStates[it] = make_pair(state, isFinal);
                    isAdded = true;
                    break;
                }
            }

            if (!isAdded) {
                tmpStates[it] = make_pair(it, isFinal);
                boolMap[isFinal].insert(it);
            }
        }

        if (tmpStates == newStates) {
            break;
        } else {
            newStates = tmpStates;
            newTransitions.clear();

            for (const auto &state: a.m_States) {
                for (const auto &alpha: a.m_Alphabet) {
                    if (a.m_Transitions.at(make_pair(state, alpha)) == 1024)
                        newTransitions[state].push_back(1024);
                    else
                        newTransitions[state].push_back(
                                newStates.at(a.m_Transitions.at(make_pair(state, alpha))).first);
                }
            }
        }
    }

    DFA newAutomata{
            {},
            {a.m_Alphabet},
            {},
            a.m_InitialState,
            {}
    };

    for (const auto &finalState: boolMap[true]) {
        newAutomata.m_FinalStates.insert(finalState);
    }

    for (const auto &it: newStates) {
        newAutomata.m_States.insert(it.second.first);
    }

    for (const auto &state: newAutomata.m_States) {
        int i = 0;
        for (const auto &alpha: newAutomata.m_Alphabet) {
            if (newTransitions[state][i] != 1024)
                newAutomata.m_Transitions.insert({{state, alpha}, newTransitions[state][i]});
            i++;
        }
    }

    return newAutomata;
}

DFA unify(const NFA &a, const NFA &b) {
    set<Symbol> newAlpha;
    for (const auto &it: a.m_Alphabet) {
        newAlpha.insert(it);
    }
    for (const auto &it: b.m_Alphabet) {
        newAlpha.insert(it);
    }

    DFA dfaA = determize({{a.m_States}, newAlpha, {a.m_Transitions}, a.m_InitialState, {a.m_FinalStates}});
    DFA dfaB = determize({{b.m_States}, newAlpha, {b.m_Transitions}, b.m_InitialState, {b.m_FinalStates}});
    DFA newA = fullDFA(dfaA);
    DFA newB = fullDFA(dfaB);

    DFA renamedB = renameAutomat(newA, newB);
    DFA unionAutomat = parallelRun(newA, renamedB, true);
    DFA automataWithoutUselessStates = deleteUselessStates(unionAutomat);
    return automataWithoutEquivalentStates(automataWithoutUselessStates);
}

DFA intersect(const NFA &a, const NFA &b) {
    set<Symbol> newAlpha;
    for (const auto &it: a.m_Alphabet) {
        newAlpha.insert(it);
    }
    for (const auto &it: b.m_Alphabet) {
        newAlpha.insert(it);
    }

    DFA dfaA = determize({{a.m_States}, newAlpha, {a.m_Transitions}, a.m_InitialState, {a.m_FinalStates}});
    DFA dfaB = determize({{b.m_States}, newAlpha, {b.m_Transitions}, b.m_InitialState, {b.m_FinalStates}});
    DFA newA = fullDFA(dfaA);
    DFA newB = fullDFA(dfaB);

    DFA renamedB = renameAutomat(newA, newB);
    DFA unionAutomat = parallelRun(newA, renamedB, false);
    DFA automataWithoutUselessStates = deleteUselessStates(unionAutomat);
    return automataWithoutEquivalentStates(automataWithoutUselessStates);
}

#ifndef __PROGTEST__

// You may need to update this function or the sample data if your state naming strategy differs.
bool operator==(const DFA &a, const DFA &b) {
    return std::tie(a.m_States, a.m_Alphabet, a.m_Transitions, a.m_InitialState, a.m_FinalStates) ==
           std::tie(b.m_States, b.m_Alphabet, b.m_Transitions, b.m_InitialState, b.m_FinalStates);
}

int main() {
    NFA a1{
            {0, 1, 2},
            {'a', 'b'},
            {
                    {{0, 'a'}, {0, 1}},
                    {{0, 'b'}, {0}},
                    {{1, 'a'}, {2}},
            },
            0,
            {2},
    };
    NFA a2{
            {0, 1, 2},
            {'a', 'b'},
            {
                    {{0, 'a'}, {1}},
                    {{1, 'a'}, {2}},
                    {{2, 'a'}, {2}},
                    {{2, 'b'}, {2}},
            },
            0,
            {2},
    };
    DFA a{
            {0, 1, 2, 3, 4},
            {'a', 'b'},
            {
                    {{0, 'a'}, {1}},
                    {{1, 'a'}, {2}},
                    {{2, 'a'}, {2}},
                    {{2, 'b'}, {3}},
                    {{3, 'a'}, {4}},
                    {{3, 'b'}, {3}},
                    {{4, 'a'}, {2}},
                    {{4, 'b'}, {3}},
            },
            0,
            {2},
    };
//    assert(intersect(a1, a2) == a);

    NFA b1{
            {0, 1, 2, 3, 4},
            {'a', 'b'},
            {
                    {{0, 'a'}, {1}},
                    {{0, 'b'}, {2}},
                    {{2, 'a'}, {2, 3}},
                    {{2, 'b'}, {2}},
                    {{3, 'a'}, {4}},
            },
            0,
            {1, 4},
    };
    NFA b2{
            {0, 1, 2, 3, 4},
            {'a', 'b'},
            {
                    {{0, 'b'}, {1}},
                    {{1, 'a'}, {2}},
                    {{2, 'b'}, {3}},
                    {{3, 'a'}, {4}},
                    {{4, 'a'}, {4}},
                    {{4, 'b'}, {4}},
            },
            0,
            {4},
    };
    DFA b{
            {0, 1, 2, 3, 4, 5, 6, 7, 8},
            {'a', 'b'},
            {
                    {{0, 'a'}, {1}},
                    {{0, 'b'}, {2}},
                    {{2, 'a'}, {3}},
                    {{2, 'b'}, {4}},
                    {{3, 'a'}, {5}},
                    {{3, 'b'}, {6}},
                    {{4, 'a'}, {7}},
                    {{4, 'b'}, {4}},
                    {{5, 'a'}, {5}},
                    {{5, 'b'}, {4}},
                    {{6, 'a'}, {8}},
                    {{6, 'b'}, {4}},
                    {{7, 'a'}, {5}},
                    {{7, 'b'}, {4}},
                    {{8, 'a'}, {8}},
                    {{8, 'b'}, {8}},
            },
            0,
            {1, 5, 8},
    };
//    assert(unify(b1, b2) == b);

    NFA c1{
            {0, 1, 2, 3, 4},
            {'a', 'b'},
            {
                    {{0, 'a'}, {1}},
                    {{0, 'b'}, {2}},
                    {{2, 'a'}, {2, 3}},
                    {{2, 'b'}, {2}},
                    {{3, 'a'}, {4}},
            },
            0,
            {1, 4},
    };
    NFA c2{
            {0, 1, 2},
            {'a', 'b'},
            {
                    {{0, 'a'}, {0}},
                    {{0, 'b'}, {0, 1}},
                    {{1, 'b'}, {2}},
            },
            0,
            {2},
    };
    DFA c{
            {0},
            {'a', 'b'},
            {},
            0,
            {},
    };
//    assert(intersect(c1, c2) == c);

    NFA d1{
            {0, 1, 2, 3},
            {'i', 'k', 'q'},
            {
                    {{0, 'i'}, {2}},
                    {{0, 'k'}, {1, 2, 3}},
                    {{0, 'q'}, {0, 3}},
                    {{1, 'i'}, {1}},
                    {{1, 'k'}, {0}},
                    {{1, 'q'}, {1, 2, 3}},
                    {{2, 'i'}, {0, 2}},
                    {{3, 'i'}, {3}},
                    {{3, 'k'}, {1, 2}},
            },
            0,
            {2, 3},
    };
    NFA d2{
            {0, 1, 2, 3},
            {'i', 'k'},
            {
                    {{0, 'i'}, {3}},
                    {{0, 'k'}, {1, 2, 3}},
                    {{1, 'k'}, {2}},
                    {{2, 'i'}, {0, 1, 3}},
                    {{2, 'k'}, {0, 1}},
            },
            0,
            {2, 3},
    };
    DFA d{
            {0, 1, 2, 3},
            {'i', 'k', 'q'},
            {
                    {{0, 'i'}, {1}},
                    {{0, 'k'}, {2}},
                    {{2, 'i'}, {3}},
                    {{2, 'k'}, {2}},
                    {{3, 'i'}, {1}},
                    {{3, 'k'}, {2}},
            },
            0,
            {1, 2, 3},
    };
//    assert(intersect(d1, d2) == d);
}

#endif