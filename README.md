# 2º Trabalho Redes
**Implementação de um IRC, ou _Internet Chat Relay_**

## Integrantes

- Arthur Vergaças | 12542672
- Benício Januário | 12543843
- Henrique Bovo | 12542539

## Executando o projeto

O projeto foi desenvolvido e testado no seguinte ambiente:

- **Sistema Operacional**:  Ubuntu 22.04.1 LTS | Windows Subsystem for Linux (WSL2)
- **Compilador**: gcc version 11.3.0 (Ubuntu 11.3.0-1ubuntu1~22.04)

No entanto, o projeto também foi testado em outros ambientes, e portanto seguir a risca estas especificações não é obrigatório.

Para rodar o programa, você deve executar em um terminal **Linux** com `gcc` e `make` instalados os seguintes comandos:

### Para rodar o servidor

```sh
$ make run-server
```

### Para rodar o cliente

```sh
$ make run-client
```

O servidor roda no host `127.0.0.1` (o `localhost`), e sua porta é a `3000`. Para mudar estas configurações, é necessário alterar os seguintes valores no arquivo `include/server.h`:

- `#define SERVER_IP '127.0.0.1'`
- `#define SERVER_PORT 3000`

## Apresentação do trabalho

Um vídeo foi gravado com o objetivo de apresentar as funcionalidades do programa e explicar seus principais pontos.

**[Link do vídeo.](https://drive.google.com/file/d/1Tb_WbOAj_tSUUEJfei95to8fstoVWxTu/view?usp=sharing)**
