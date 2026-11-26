# Roadmap

Projeto de código aberto sob GPLv3.

Estado real de cada fase. Atualizar apenas quando o item estiver verificado.

## Fase 0 — Decisões e fundação (em curso)

- [x] Escolher licença do projeto: GPLv3 (repositório público).
- [ ] Adicionar o ficheiro LICENSE (GPLv3) ao repositório.
- [ ] Confirmar a versão de JUCE e os termos GPLv3 aplicáveis.
- [ ] Decidir se o Tracktion Engine entra no projeto.
- [ ] Confirmar licença do VST3 SDK e compatibilidade com GPLv3.
- [x] Criar repositório no GitHub.
- [x] Criar esqueleto CMake que compila uma janela vazia no Windows.
- [x] Verificar compilação em Visual Studio 2022 (GitHub Actions, windows-2022).

## Milestone 001 — Audio Workstation Foundation

- [x] Aplicação abre sem erros e mostra a janela principal.
- [x] Saída de áudio padrão do sistema (dispositivo por defeito; seleção de dispositivo e ASIO por fazer).
- [x] Importar WAV, AIFF, FLAC e OGG (MP3 por fazer).
- [x] Reproduzir e parar áudio, com volume e tempo.
- [ ] Criar e remover pistas.
- [x] Mostrar formas de onda, com cursor de reprodução e clique para saltar de posição.
- [ ] Guardar e abrir projetos.

## Milestone 002 — Plugin Hosting

- [ ] Pesquisar plugins VST3 instalados.
- [ ] Carregar instrumentos VST3 e reproduzir MIDI através deles.
- [ ] Carregar efeitos VST3.
- [ ] Abrir a interface original do plugin.
- [ ] Guardar e restaurar o estado dos plugins.

## Milestone 003 — Universal Mixer

- [ ] Mixer funcional com volume, pan, mute e solo.
- [ ] Inserts e cadeias de efeitos.
- [ ] Routing entre canais.
- [ ] Acesso aos mesmos plugins a partir da Playlist, do Mixer e de janelas independentes.

## Fora do escopo por agora

- VST2.
- Plugins nativos da casa.
- Qualquer nome comercial definitivo.
