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

## Milestone 002 — Janelas do estúdio (em curso)

- [x] Modelo de projeto partilhado entre todas as janelas (`Source/Model/ProjectModel.h`).
- [x] Timeline: arranjo com régua, faixas e clips de 1 compasso.
- [x] Console: tiras por canal com fader, pan, mute e solo, e fader master.
- [x] Beat Deck: grelha de 16 passos por canal, com M, S e adição de canais.
- [x] Note Canvas: editor de notas do canal selecionado.
- [x] Plugin Vault: scan de VST3 em segundo plano, com lista guardada em disco.
- [ ] Reprodução dos passos do Beat Deck e dos clips da Timeline (ligar ao motor).
- [ ] Medidores ao vivo no Console.
- [ ] Guardar e abrir projetos (JSON).

## Milestone 003 — Console escalável (em curso)

- [x] Console com 20 canais iniciais e até 1000 canais, com scroll horizontal.
- [x] Pool de tiras reciclado: só existem componentes para o que está no ecrã.
- [x] Rack de efeitos por canal: 50 slots, 10 visíveis com scroll, preenchidos a partir do Plugin Vault.
- [x] Beat Deck e Timeline com scroll vertical e desenho só das linhas visíveis.
- [x] Lista de plugins partilhada (modelo) entre Plugin Vault e Console.
- [ ] Processamento real: passar os slots de efeito ao motor de áudio.
- [ ] Reprodução dos passos e dos clips.
- [ ] Medidores ao vivo.
- [ ] Atualizar para JUCE 9 (após migração de API).

## Milestone 004 — Projetos e biblioteca (em curso)

- [x] Formato de projeto `.opal` (XML versionado, validado ao abrir, escrita segura com ficheiro temporário).
- [x] Novo, Abrir, Guardar e Guardar como, com atalhos Ctrl+N, Ctrl+O, Ctrl+S e Ctrl+Shift+S.
- [x] Título da janela mostra o nome do projeto e • quando há alterações por guardar.
- [x] Cópia de recuperação automática a cada minuto (pasta de dados do Opalo).
- [x] Browser com a pasta de kits (Documentos/Opalo/Kits): atribui sons aos canais.
- [x] Importação de notas MIDI para o canal selecionado (dentro dos 16 passos).
- [x] Playlist redesenhada: régua com compassos fortes, zebra, clips com contorno e nome, hover.
- [ ] Abrir projetos de outras DAWs (ver nota abaixo).
- [x] Lista de projetos recentes (menu do botão Open).
- [x] Desfazer e refazer (Ctrl+Z, Ctrl+Y ou Ctrl+Shift+Z), até 100 passos, com alterações seguidas agrupadas.
- [ ] Carregamento de samples dentro do motor.

Nota sobre outras DAWs: os formatos nativos do FL Studio (.flp), Studio One, Pro Tools e Ableton são proprietários e
não documentados, e mudam entre versões. Ler esses ficheiros diretamente não é fiável nem recomendado. O caminho
seguro é importar por formatos abertos: MIDI (já implementado), áudio (WAV, FLAC) e, mais tarde, AAF ou OMF.

## Milestone 005 — Idiomas, áudio, mashup e corte (em curso)

- [x] Idiomas: Português, English e Français, escolhidos nas Definições e guardados no perfil do utilizador.
- [x] Definições: idioma, dispositivo de áudio (seletor do JUCE) e tamanho do buffer (64 a 1024), com latência estimada em ms.
- [x] Estado do dispositivo de áudio guardado e restaurado ao arrancar.
- [x] Mashup: dois decks com carregamento, BPM editável, varispeed (muda velocidade e tom), crossfader com potência constante, sincronizar com o projeto e igualar B a A.
- [x] Ferramenta de corte na Playlist: atalho C para cortar e D para selecionar, com ícone de lâmina próprio.
- [ ] ASIO: exige o SDK da Steinberg (download à parte, licença própria). Quando for adicionado, o ASIO4ALL aparece no seletor de dispositivos. Até lá, usar WASAPI.
- [ ] Time-stretch (mudar o tempo sem mudar o tom). Por agora só existe varispeed.
- [ ] Tradução dos títulos dos painéis ao mudar de idioma (só no arranque).
- [ ] Efeitos e instrumentos VST3 a processar áudio no canal.
- [ ] Reprodução dos passos do Beat Deck e dos clips da Playlist.

## Design (em curso)

- [x] Tokens de design centralizados (cores em camadas, raios, espaçamentos, tipos de letra).
- [x] Botões com estados: normal, hover, pressionado, ativo e desativado.
- [x] Faders horizontais e verticais com preenchimento em acento e polegar com estado de hover.
- [x] Caixas de seleção e menus com as mesmas cores; dicas (tooltips) no mesmo estilo.
- [x] Janelas com barra de título do Opalo: minimizar, maximizar e fechar no canto superior esquerdo.
- [ ] Ícones vetoriais próprios para os botões de transporte e de painel.
- [ ] Revisão visual no ecrã real (cores, contrastes e alinhamentos) com o primeiro build.

## Identidade visual

- [x] Marca Opalo (gema de opala com brilho iridescente e nome em letras espaçadas).
- [x] Ecrã de arranque animado.
- [ ] Ícone do executável e da barra de tarefas.

## Fora do escopo por agora

- VST2.
- Plugins nativos da casa.
- Qualquer nome comercial definitivo.
