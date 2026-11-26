#pragma once

#include <juce_core/juce_core.h>
#include <vector>

// Desfazer e refazer por snapshots do projeto (XML). Guarda no máximo 100 passos.
class ProjectHistory
{
public:
    static constexpr size_t maxSteps = 100;

    // Define o estado de partida (novo projeto, projeto aberto ou guardado).
    void reset(const juce::String& snapshot)
    {
        current = snapshot;
        undoStack.clear();
        redoStack.clear();
    }

    // Regista um novo estado. Ignora-o se for igual ao atual.
    bool record(const juce::String& snapshot)
    {
        if (snapshot == current)
            return false;

        undoStack.push_back(current);
        if (undoStack.size() > maxSteps)
            undoStack.erase(undoStack.begin());

        current = snapshot;
        redoStack.clear();
        return true;
    }

    bool canUndo() const { return ! undoStack.empty(); }
    bool canRedo() const { return ! redoStack.empty(); }

    // Devolve o snapshot a restaurar, ou uma string vazia se não houver nada.
    juce::String undo()
    {
        if (undoStack.empty())
            return {};

        redoStack.push_back(current);
        current = undoStack.back();
        undoStack.pop_back();
        return current;
    }

    juce::String redo()
    {
        if (redoStack.empty())
            return {};

        undoStack.push_back(current);
        current = redoStack.back();
        redoStack.pop_back();
        return current;
    }

private:
    juce::String current;
    std::vector<juce::String> undoStack;
    std::vector<juce::String> redoStack;
};
