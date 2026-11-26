# CLAUDE.md — Regras do projeto

Este ficheiro é a autoridade do projeto. Lê-o antes de qualquer alteração.

## Identidade do projeto

- DAW profissional para Windows 10/11 x64, de código aberto sob GPLv3.
- Repositório público. Pode ser vendido em binários ou com serviços, mas o código-fonte
  tem de continuar disponível para quem recebe a aplicação (exigência da GPLv3).
- **Nome final ainda não definido.** Usar o nome provisório `DAWWorkstation`
  (namespace, nomes de projeto e de target). Não introduzir outros nomes de produto.
- Comunicação com o developer em português. Identificadores C++, nomes de ficheiros e
  convenções técnicas em inglês.

## Stack (proposta, sujeita a confirmação de licenças)

- C++20
- JUCE, usada sob GPLv3 (versão a confirmar na documentação oficial antes de fixar)
- Tracktion Engine, também GPLv3, compatível com o projeto (integração a decidir na Fase 0)
- Steinberg VST3 SDK (versão e licença a confirmar, incluindo compatibilidade com GPLv3)
- CMake
- Visual Studio 2022 / MSVC

Antes de adicionar qualquer dependência, verificar versão atual, licença e compatibilidade
com GPLv3. Não usar código de licença incompatível, nem código copiado sem licença clara.

## Regras obrigatórias

1. Não substituir a arquitetura sem autorização explícita do developer.
2. Não criar plugins de áudio nativos nesta fase.
3. Não simular funcionalidades: tudo o que aparece na interface deve funcionar.
4. Não criar botões decorativos sem comportamento.
5. Um plugin existe como uma única instância de processamento. Mixer, Playlist e janela
   independente são apenas vistas dessa mesma instância. Nunca duplicar instâncias.
6. Nunca fazer I/O de disco, alocação pesada, bloqueios ou operações de GUI no callback de áudio.
7. Não alterar módulos estáveis sem necessidade.
8. Não entregar código sem verificar que compila.
9. Não avançar de fase sem testar a fase anterior.
10. Não adicionar dependências sem verificar a sua licença.
11. Priorizar estabilidade, desempenho e qualidade de áudio.
12. O projeto está sob GPLv3. Todo o código novo deve ter o cabeçalho de copyright adequado.
    Não misturar código de licença incompatível com a GPLv3.

## Como trabalhar

- Antes de gerar muito código, apresentar o plano técnico.
- Fazer commits em marcos estáveis.
- Manter `Documentation/ROADMAP.md` atualizado com o estado real de cada fase.
- Manter `LICENSES/THIRD_PARTY_LICENSES.md` atualizado a cada dependência nova.
