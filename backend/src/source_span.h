#pragma once

#include "token_stream.h"

// SourceSpan identifica un intervalo del archivo fuente.
// Las posiciones finales son exclusivas: [inicio, fin).
// Mantener la ubicación en el árbol permite que las fases posteriores
// produzcan diagnósticos sin volver a inspeccionar el texto original.
struct SourceSpan {
    int start_line = 0;
    int start_column = 0;
    int end_line = 0;
    int end_column = 0;

    bool valid() const {
        return start_line > 0 && start_column > 0;
    }

    static SourceSpan fromToken(const Token& token) {
        SourceSpan span;
        span.start_line = token.linea;
        span.start_column = token.columna;
        span.end_line = token.linea;
        span.end_column = token.columna;

        for (char c : token.lexema) {
            if (c == '\n') {
                span.end_line++;
                span.end_column = 1;
            } else {
                span.end_column++;
            }
        }
        return span;
    }

    static SourceSpan covering(const SourceSpan& first,
                               const SourceSpan& last) {
        if (!first.valid()) return last;
        if (!last.valid()) return first;
        return {first.start_line, first.start_column,
                last.end_line, last.end_column};
    }
};
