# Cracktro - Remake Mapeno

Uma recriação de um "cracktro" clássico, desenvolvida em C++ utilizando a API do Windows.

## Funcionalidades

*   **Reprodução de Música:** Suporte para ficheiros de módulo (.MOD) através da biblioteca Mod4Play.
*   **Efeitos Visuais:** Renderização de efeitos gráficos em tempo real.
*   **Gráficos:** Utilização de GDI+ para desenho de alta qualidade.
*   **Áudio:** Integração com a API WinMM para saída de som.

## Requisitos

*   Windows 10 ou superior.
*   Visual Studio 2022 (com suporte para C++ e ferramentas de build MSVC).

## Como Compilar

O projeto inclui um script de build simples para facilitar a compilação.

1.  Abra a linha de comandos (Command Prompt).
2.  Navegue até à diretoria do projeto.
3.  Execute o script de build:

    ```cmd
    build.bat
    ```

4.  Após a compilação bem-sucedida, o executável `cracktro.exe` será gerado.

## Funcionamento Interno

Esta aplicação demonstra várias técnicas de programação de sistemas e computação gráfica "old-school".

### Ciclo Principal e Janela
A aplicação utiliza um ciclo de mensagens Windows padrão (`PeekMessage`) para processar eventos. O controlo de tempo é feito através de `GetTickCount` para calcular o `deltaTime`, garantindo que as animações correm à mesma velocidade independentemente da taxa de atualização do monitor ou velocidade do processador.

### Sistema Gráfico Híbrido
O motor gráfico combina duas abordagens distintas para maximizar a performance e qualidade visual:

1.  **Pixel Buffer Direto (DIB Section):**
    *   Utilizado para os efeitos de fundo (campo de estrelas e "copper bars").
    *   Permite manipulação direta de memória (`g_pBits`) para desenhar pixel a pixel, essencial para a performance do efeito de estrelas 3D.
    *   As "Copper Bars" são geradas matematicamente usando funções seno para criar gradientes suaves e movimento orgânico.

2.  **GDI+:**
    *   Utilizado para elementos de "alto nível" como o logótipo e o texto em scroll.
    *   Permite carregamento fácil de imagens PNG (com transparência) a partir dos recursos.
    *   O "Scroller" (texto em movimento) utiliza transformações de matriz do GDI+ para rodar cada letra individualmente, acompanhando a curva sinusoidal do movimento.

### Sistema de Áudio
A reprodução de música é gerida por um sistema de duplo buffer para garantir áudio sem falhas:

*   **Mod4Play:** O "core" do leitor, responsável por misturar os samples do módulo (.MOD) num buffer PCM.
*   **WinMM:** A API multimédia do Windows (`waveOut`) é usada para enviar o áudio para a placa de som.
*   **Buffering:** São utilizados 3 buffers de 8192 samples cada. Enquanto um buffer está a ser reproduzido, o próximo está a ser preenchido pelo Mod4Play em background.

### Recursos
Todos os assets (música e imagens) estão embutidos no executável através de ficheiros de recursos (`.rc`), permitindo que a aplicação seja distribuída como um único ficheiro `.exe` sem dependências externas.


## Créditos

Este projeto utiliza as seguintes bibliotecas e recursos:

*   **Mod4Play:** Biblioteca de reprodução de módulos de música (IT/S3M/XM/MOD/FT), criada por **dashodanger** e **Olav Sørensen**.

## Licença

Este projeto é disponibilizado para fins educacionais e de demonstração.
