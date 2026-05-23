#include "first_follow.h"
#include <iostream>
#include <iomanip>

const std::string FirstFollow::EPSILON = "ε";
const std::string FirstFollow::END     = "$";

void FirstFollow::compute(const Grammar& g) {
    first_.clear();
    follow_.clear();

    // FIRST de terminales es el terminal mismo
    for (const auto& t : g.terminals)
        first_[t] = { t };
    first_[END]     = { END };
    first_[EPSILON] = { EPSILON };

    // FIRST vacío para no terminales
    for (const auto& nt : g.non_terminals)
        if (first_.find(nt) == first_.end())
            first_[nt] = {};

    computeFirst(g);
    computeFollow(g);
}

// ── FIRST ─────────────────────────────────────────────────

void FirstFollow::computeFirst(const Grammar& g) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : g.productions) {
            const std::string& A = prod.head;
            auto& firstA = first_[A];
            size_t before = firstA.size();

            // FIX 2: producción vacía A → ε
            if (prod.body.empty() || prod.body[0] == "") {
                firstA.insert(EPSILON);
            } else {
                bool allNullable = true;
                for (const auto& Yi : prod.body) {
                    auto it = first_.find(Yi);
                    if (it != first_.end()) {
                        // agrega FIRST(Yi) - {ε}
                        for (const auto& s : it->second)
                            if (s != EPSILON) firstA.insert(s);
                        // si Yi NO puede derivar ε, para
                        if (!it->second.count(EPSILON)) {
                            allNullable = false;
                            break;
                        }
                    } else {
                        allNullable = false;
                        break;
                    }
                }
                // si todos los Yi pueden derivar ε → agrega ε
                if (allNullable) firstA.insert(EPSILON);
            }

            if (firstA.size() != before) changed = true;
        }
    }
}

// ── FOLLOW ────────────────────────────────────────────────

void FirstFollow::computeFollow(const Grammar& g) {
    for (const auto& nt : g.non_terminals)
        follow_[nt] = {};

    // Regla 1: $ en FOLLOW del símbolo inicial
    follow_[g.start_symbol].insert(END);

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : g.productions) {
            const std::string& A = prod.head;

            for (size_t i = 0; i < prod.body.size(); i++) {
                const std::string& B = prod.body[i];
                if (!g.isNonTerminal(B)) continue;

                auto& followB = follow_[B];
                size_t before = followB.size();

                // β = lo que viene después de B
                std::vector<std::string> beta(
                    prod.body.begin() + i + 1,
                    prod.body.end());

                // FIX 2: si β está vacío o toda β deriva ε
                // agrega FOLLOW(A) a FOLLOW(B)
                if (beta.empty()) {
                    for (const auto& s : follow_[A])
                        followB.insert(s);
                } else {
                    auto firstBeta = firstOfSequence(beta);

                    // Regla 2: agrega FIRST(β) - {ε}
                    for (const auto& s : firstBeta)
                        if (s != EPSILON) followB.insert(s);

                    // Regla 3: si ε ∈ FIRST(β), agrega FOLLOW(A)
                    if (firstBeta.count(EPSILON))
                        for (const auto& s : follow_[A])
                            followB.insert(s);
                }

                if (followB.size() != before) changed = true;
            }
        }
    }
}

// ── firstOfSequence ───────────────────────────────────────

std::set<std::string> FirstFollow::firstOfSequence(
    const std::vector<std::string>& seq) const {

    std::set<std::string> result;
    if (seq.empty()) { result.insert(EPSILON); return result; }

    bool allNullable = true;
    for (const auto& Yi : seq) {
        // FIX 2: producción vacía en la secuencia
        if (Yi == "") { continue; }

        auto it = first_.find(Yi);
        if (it != first_.end()) {
            for (const auto& s : it->second)
                if (s != EPSILON) result.insert(s);
            if (!it->second.count(EPSILON)) {
                allNullable = false;
                break;
            }
        } else {
            allNullable = false;
            break;
        }
    }
    if (allNullable) result.insert(EPSILON);
    return result;
}

// ── Getters ───────────────────────────────────────────────

const std::set<std::string>& FirstFollow::first(
    const std::string& symbol) const {
    static std::set<std::string> empty;
    auto it = first_.find(symbol);
    return (it != first_.end()) ? it->second : empty;
}

const std::set<std::string>& FirstFollow::follow(
    const std::string& symbol) const {
    static std::set<std::string> empty;
    auto it = follow_.find(symbol);
    return (it != follow_.end()) ? it->second : empty;
}

// ── Print ─────────────────────────────────────────────────

void FirstFollow::print() const {
    std::cout << "\n========== CONJUNTOS FIRST ==========\n";
    std::cout << std::setw(15) << std::left << "No terminal"
              << "  FIRST\n";
    std::cout << std::string(50, '-') << "\n";
    for (const auto& [sym, fset] : first_) {
        if (!std::islower((unsigned char)sym[0])) continue;
        std::cout << std::setw(15) << std::left << sym << "  { ";
        for (const auto& s : fset) std::cout << s << " ";
        std::cout << "}\n";
    }

    std::cout << "\n========== CONJUNTOS FOLLOW ==========\n";
    std::cout << std::setw(15) << std::left << "No terminal"
              << "  FOLLOW\n";
    std::cout << std::string(50, '-') << "\n";
    for (const auto& [sym, fset] : follow_) {
        if (sym.empty()) continue;
        std::cout << std::setw(15) << std::left << sym << "  { ";
        for (const auto& s : fset) std::cout << s << " ";
        std::cout << "}\n";
    }
    std::cout << "======================================\n\n";
}
