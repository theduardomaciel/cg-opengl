<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/.github/cover.png">
  <source media="(prefers-color-scheme: light)" srcset="/.github/cover_light.png">
    <img alt="Banner do projeto" src="/.github/cover_light.png">
</picture>

<br />

## 🚶 Sobre o Projeto

Um simulador minimalista em primeira pessoa para explorar o Memorial à República em Maceió–AL como requisito da disciplina de Computação Gráfica (UFAL, 2025.1).  
Construído com OpenGL (3.3+), C++, e CMake.

> [!WARNING]
> Alguns arquivos grandes, como modelos OBJ, não estão inclusos no repositório por questões de tamanho.  
> Para baixá-los, basta acessar a [pasta no Drive do projeto](https://drive.google.com/drive/folders/1HmuZ-jdn9_bICnKb1MeXOPKZ0eIS5S1N?usp=sharing) e colocar os arquivos na pasta `models/` na raiz do projeto.

---

## ⭐ Features
- Janela GLFW + captura de entrada
- Câmera estilo FPS (WASD + olhar com o mouse)
- Alternar captura do cursor (tecla `C`)
- **Sistema robusto de carregamento de modelos OBJ**
- **Renderização 3D com iluminação básica (Phong)**
- **Modelo do centro histórico carregado automaticamente**
- Modo wireframe alternável (Ctrl + W)
- FPS no título da janela + VSync habilitado
- Shaders modernos com suporte a normais e materiais

---

## ⚙️ Arquitetura Modular
A aplicação foi pensada para ter um layout de engine simples e extensível:

```
include/
  core/      -> Application, Window
  input/     -> Camera, Input helpers
  render/    -> Shader, Mesh, Model, ModelLoader, Renderer (sistema completo de renderização 3D)
  physics/   -> PhysicsSystem (stub p/ Bullet)
  ui/        -> DebugUI (stub p/ ImGui)
src/
  core/, input/, render/, physics/, ui/
models/
  structure_v{x}.obj -> Modelo principal do centro histórico
```

### Principais conceitos:
- Application: orquestra loop principal (input -> física -> UI -> render).
- Sistemas desacoplados: cada domínio em pasta própria para evolução incremental.
- **Sistema de renderização modular**: Mesh (geometria), Model (transformações), ModelLoader (carregamento OBJ), Renderer (coordenação da renderização)
- **Carregamento robusto de modelos**: Parser completo de arquivos OBJ com suporte a múltiplas meshes, normais automáticas, e otimização de vértices
- **Iluminação Phong**: Shader avançado com componentes ambiente, difusa e especular
- Fácil expansão: adicionar novo módulo = criar headers em `include/<mod>/` + fontes e registrar no `CMakeLists.txt` (lista `CG_ENGINE_SOURCES`).
- Stubs (PhysicsSystem, DebugUI) permitem integrar Bullet / ImGui sem alterar o loop.

### Extensão rápida:
1. Criar arquivos em `include/novo/NovoSistema.h` e `src/novo/NovoSistema.cpp`.
2. Incluir no `CMakeLists.txt` em `CG_ENGINE_SOURCES`.
3. Instanciar em `Application` (campo + init + chamada no loop se necessário).
4. (Opcional) Adicionar dependências externas com `add_subdirectory` ou `find_package`.

---

## 🖇️ Estrutura de Dependências
- GLFW: criação de janela e entrada
- GLAD: carregamento de funções OpenGL 3.3 Core
- GLM: matemática (vec/mat/projeção)
- Assimp: carregamento de modelos 3D (formato OBJ)
- Bullet (stub): física básica

> [!NOTE]
> Outras libs (ImGui) poderão ser integradas em versões futuras.

### Diretórios relevantes
```
external/glfw
external/glm
external/glad (gerador Python)
external/glad-build (código gerado: include + src/glad.c)
```

Se o diretório `external/glad-build` não existir (por ex. após um clone limpo), gere o GLAD:

```bash
# Requer Python 3 e acesso à internet para baixar especificações
python -m glad --profile=core --api="gl=3.3" --generator=c --out-path=external/glad-build
```

> [!NOTE]
> Certifique-se de que o Python esteja no PATH do sistema.
> No Windows PowerShell, caso falhe: `setx PYTHONPATH external` ou use ambiente virtual; rode alternativamente a partir da raiz: `python external/glad/glad/__main__.py ...`)

---

## 🛠️ Setup & Build

### 1. Clonar repositório
```bash
git clone https://github.com/theduardomaciel/cg-opengl.git
cd cg-opengl
```

### 2. Adicionar (ou atualizar) submódulos quando necessário
Se ainda não tiver as libs, adicione (ou use `git submodule add` conforme necessidade). Para sincronizar existentes:
```bash
git submodule update --init --recursive
```

### 3. (Opcional) Gerar GLAD se ausente
(Ver seção acima.)

### 4. Configurar e compilar

#### Linux / macOS
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

#### Windows (Visual Studio / MSVC)
```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```
Binário resultante (MSVC): `build/Debug/cg_opengl.exe`

#### Windows (MinGW)
```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```
Binário: `build/cg_opengl.exe`

### 5. Executar
```bash
./build/cg_opengl        # Linux/macOS ou MinGW
./build/Debug/cg_opengl  # MSVC
```

---

## 🕹️ Controles
* `W / A / S / D` → movimentação no plano (sem voo)
* Mouse → olhar (pitch limitado ±89°)
* `C` → alterna captura do cursor (lock/unlock)
* `Ctrl + W` → alterna modo wireframe/sólido
* `ESC` → sair

---

## 👣 Próximos Passos (Roadmap)
1. [x] Carregamento de modelos (sistema OBJ completo implementado)
2. [x] Sistema de iluminação básica (Phong implementado)
3. [x] Carregamento de texturas e materiais
4. [ ] Colisão / física básica (Bullet)
5. [ ] UI de debug (ImGui)
6. [ ] Sistema de materiais avançado com texturas
7. [ ] Altura do terreno / navegação com colisão

---

## 🐞 Ferramentas de Debug
* [RenderDoc](https://renderdoc.org) → inspeção de frames
* `glfwSetErrorCallback` → log de erros de contexto
* Compilar com flags de warning altas (já configurado)

Para logging extra, adicionar prints ou macro de debug.
