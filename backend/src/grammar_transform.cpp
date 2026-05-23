#include "grammar_transform.h"
#include <algorithm>
#include <map>
#include <set>

using Body = std::vector<std::string>;
using Alts = std::vector<Body>;

// ── Utilidades internas ───────────────────────────────────────

static bool isEps(const Body& b) {
    return b.empty() || (b.size() == 1 && b[0].empty());
}

static Body makeEps() { return {""}; }

// Genera un nombre no presente en 'used'. Intenta base', base'', ...
static std::string freshName(const std::string& base,
                               const std::set<std::string>& used) {
    std::string name = base + "'";
    while (used.count(name)) name += "'";
    return name;
}

// Agrupa producciones por cabeza, respetando el orden de non_terminals.
static void groupProds(const Grammar& g,
                        std::vector<std::string>& order,
                        std::map<std::string, Alts>& rules) {
    order = g.non_terminals;
    for (const auto& p : g.productions)
        rules[p.head].push_back(p.body);
}

// Reconstruye Grammar a partir del orden de NTs y las reglas.
static Grammar rebuild(const std::vector<std::string>& ntOrder,
                        const std::map<std::string, Alts>& rules,
                        const Grammar& orig) {
    Grammar g;
    g.terminals    = orig.terminals;
    g.start_symbol = orig.start_symbol;
    g.ignored      = orig.ignored;

    for (const auto& nt : ntOrder) {
        auto it = rules.find(nt);
        if (it == rules.end()) continue;
        g.non_terminals.push_back(nt);
        for (const auto& body : it->second)
            g.productions.push_back({nt, body});
    }
    return g;
}

// ── Eliminación de recursión izquierda directa ────────────────

static void elimDirectLR(const std::string& A,
                           std::map<std::string, Alts>& rules,
                           std::vector<std::string>& ntOrder,
                           std::set<std::string>& used)
{
    Alts& alts = rules[A];
    Alts recursive, nonRec;

    for (const auto& body : alts) {
        if (!isEps(body) && !body.empty() && body[0] == A)
            recursive.push_back(body);
        else
            nonRec.push_back(body);
    }

    if (recursive.empty()) return;

    std::string Ap = freshName(A, used);
    used.insert(Ap);
    ntOrder.push_back(Ap);

    // A → β A' para cada β no recursiva
    alts.clear();
    if (nonRec.empty()) {
        alts.push_back({Ap}); // caso extremo: solo recursión
    } else {
        for (auto beta : nonRec) {
            if (isEps(beta)) {
                alts.push_back({Ap});
            } else {
                beta.push_back(Ap);
                alts.push_back(beta);
            }
        }
    }

    // A' → α A' | ε para cada alternativa recursiva A → A α
    Alts& primeAlts = rules[Ap];
    for (const auto& rec : recursive) {
        Body tail(rec.begin() + 1, rec.end());
        tail.push_back(Ap);
        primeAlts.push_back(tail);
    }
    primeAlts.push_back(makeEps());
}

// ── Eliminación de recursión izquierda (directa e indirecta) ──

Grammar GrammarTransform::eliminateLeftRecursion(const Grammar& g) const {
    std::vector<std::string> ntOrder;
    std::map<std::string, Alts> rules;
    groupProds(g, ntOrder, rules);

    std::set<std::string> used(g.non_terminals.begin(), g.non_terminals.end());
    for (const auto& t : g.terminals) used.insert(t);

    int n = (int)ntOrder.size();
    for (int i = 0; i < n; i++) {
        const std::string& Ai = ntOrder[i];

        // Sustituir Aj (j < i) en las alternativas de Ai
        for (int j = 0; j < i; j++) {
            const std::string& Aj = ntOrder[j];
            Alts newAlts;
            for (const auto& body : rules[Ai]) {
                if (!isEps(body) && !body.empty() && body[0] == Aj) {
                    Body gamma(body.begin() + 1, body.end());
                    for (const auto& delta : rules[Aj]) {
                        if (isEps(delta)) {
                            newAlts.push_back(gamma.empty() ? makeEps() : gamma);
                        } else {
                            Body nb = delta;
                            nb.insert(nb.end(), gamma.begin(), gamma.end());
                            newAlts.push_back(nb);
                        }
                    }
                } else {
                    newAlts.push_back(body);
                }
            }
            rules[Ai] = newAlts;
        }

        elimDirectLR(Ai, rules, ntOrder, used);
    }

    return rebuild(ntOrder, rules, g);
}

// ── Factorización izquierda ───────────────────────────────────

static Body lcp(const Body& a, const Body& b) {
    Body result;
    size_t len = std::min(a.size(), b.size());
    for (size_t i = 0; i < len; i++) {
        if (a[i] == b[i]) result.push_back(a[i]);
        else break;
    }
    return result;
}

static Body bestPrefix(const Alts& alts) {
    Body best;
    for (size_t i = 0; i < alts.size(); i++) {
        if (isEps(alts[i])) continue;
        for (size_t j = i + 1; j < alts.size(); j++) {
            if (isEps(alts[j])) continue;
            Body p = lcp(alts[i], alts[j]);
            if (p.size() > best.size()) best = p;
        }
    }
    return best;
}

static void leftFactorNT(const std::string& A,
                          std::map<std::string, Alts>& rules,
                          std::vector<std::string>& ntOrder,
                          std::set<std::string>& used)
{
    bool changed = true;
    while (changed) {
        changed = false;
        Alts& alts = rules[A];
        Body prefix = bestPrefix(alts);
        if (prefix.empty()) break;

        Alts withPfx, without;
        for (const auto& body : alts) {
            bool matches = !isEps(body) &&
                           body.size() >= prefix.size() &&
                           Body(body.begin(), body.begin() + prefix.size()) == prefix;
            if (matches) withPfx.push_back(body);
            else         without.push_back(body);
        }
        if (withPfx.size() < 2) break;

        std::string Anew = freshName(A, used);
        used.insert(Anew);
        ntOrder.push_back(Anew);

        Body newBody = prefix;
        newBody.push_back(Anew);
        without.push_back(newBody);
        alts = without;

        Alts& newAlts = rules[Anew];
        for (const auto& body : withPfx) {
            Body tail(body.begin() + prefix.size(), body.end());
            newAlts.push_back(tail.empty() ? makeEps() : tail);
        }
        changed = true;
    }
}

Grammar GrammarTransform::leftFactor(const Grammar& g) const {
    std::vector<std::string> ntOrder;
    std::map<std::string, Alts> rules;
    groupProds(g, ntOrder, rules);

    std::set<std::string> used(g.non_terminals.begin(), g.non_terminals.end());
    for (const auto& t : g.terminals) used.insert(t);

    int orig = (int)ntOrder.size();
    for (int i = 0; i < orig; i++)
        leftFactorNT(ntOrder[i], rules, ntOrder, used);
    // Factorizar también los NTs nuevos generados
    for (int i = orig; i < (int)ntOrder.size(); i++)
        leftFactorNT(ntOrder[i], rules, ntOrder, used);

    return rebuild(ntOrder, rules, g);
}

// ── API pública ───────────────────────────────────────────────

Grammar GrammarTransform::transform(const Grammar& g) const {
    Grammar g1 = eliminateLeftRecursion(g);
    return leftFactor(g1);
}

bool GrammarTransform::hasLeftRecursion(const Grammar& g) const {
    for (const auto& p : g.productions)
        if (!p.body.empty() && p.body[0] == p.head)
            return true;
    return false;
}
