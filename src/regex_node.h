#pragma once
#include <bitset>
#include <set>
#include <string>
#include <vector>

// ─────────────────────────────────────────────
// Módulo 2 – Nodos del Árbol de Expresión (AST)
//
// Cada nodo representa un operador o hoja de la
// expresión regular. Las hojas tienen un número de
// posición único usado en la construcción directa.
// ─────────────────────────────────────────────

struct RegexNode {
    enum Type {
        LEAF,        // Hoja: coincide con algún carácter del charset
        END_MARKER,  // Hoja especial: marcador de fin de regla (#i)
        CONCAT,      // Concatenación: izq · der
        ALTER,       // Alternancia: izq | der
        STAR,        // Kleene: izq*
        PLUS,        // Positiva: izq+
        QUEST,       // Opcional: izq?
        DIFF,        // Diferencia: izq # der
    };

    Type type;

    // Para hojas (LEAF y END_MARKER)
    std::bitset<256> charset;  // conjunto de caracteres que acepta
    int  position   = -1;      // número de posición único (≥ 0)
    int  rule_index = -1;      // índice de la regla (solo para END_MARKER)
    std::string label;         // etiqueta legible para graficación

    // Subárboles
    RegexNode* left  = nullptr;
    RegexNode* right = nullptr;

    // Propiedades calculadas para construcción directa (nullable/firstpos/lastpos)
    bool            nullable = false;
    std::set<int>   firstpos;
    std::set<int>   lastpos;
};

// ─── Fábricas ─────────────────────────────────

RegexNode* makeLeaf(const std::bitset<256>& cs, int& pos_counter,
                    const std::string& lbl = "");

RegexNode* makeEndMarker(int rule_idx, int& pos_counter);

RegexNode* makeConcat(RegexNode* l, RegexNode* r);
RegexNode* makeAlter (RegexNode* l, RegexNode* r);
RegexNode* makeStar  (RegexNode* l);
RegexNode* makePlus  (RegexNode* l);
RegexNode* makeQuest (RegexNode* l);
RegexNode* makeDiff  (RegexNode* l, RegexNode* r);

// ─── Algoritmos sobre el árbol ────────────────

// Calcula nullable, firstpos y lastpos para todos los nodos (post-order)
void computePositions(RegexNode* node);

// Calcula followpos para todos los nodos
// followpos[p] = conjunto de posiciones que pueden seguir a p
void computeFollowpos(RegexNode* node,
                      std::vector<std::set<int>>& followpos);

// Construye un mapa: posición → charset del nodo hoja
void buildPosCharsetMap(RegexNode* node,
                        std::vector<std::bitset<256>>& pos_charset);

// Construye un mapa: posición → rule_index (-1 si no es END_MARKER)
void buildPosRuleMap(RegexNode* node,
                     std::vector<int>& pos_rule);

// Libera recursivamente todos los nodos
void freeTree(RegexNode* node);

// ── Funciones educativas para la exposición ──
void printTreeProperties(RegexNode* node, int depth, int limit, int& count);
void printFollowposTable(const std::vector<std::set<int>>& followpos,
                         const std::vector<int>& pos_rule);
