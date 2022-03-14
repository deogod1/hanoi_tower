#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <stack>
#include <cstdlib>
#include <ctime>
#include <mmsystem.h>

#define nullptr 0

GLUquadric* qobj;

// Variáveis para controles de navegação
GLfloat angle, fAspect;
GLfloat rotX, rotY, rotX_ini, rotY_ini;
GLfloat obsX, obsY, obsZ, obsX_ini, obsY_ini, obsZ_ini;
int x_ini,y_ini,bot;

int MAX_DISK = 3;

// Define um vértice
struct VERT
{
    float x,y,z;	// posição no espaço
};

// Define uma face
struct FACE
{
    int total;	// total de vértices
    int ind[4];	// índices para o vetor de vértices
};

// Define um objeto 3D
struct OBJ
{
    VERT *vertices;		// aponta para os vértices
    FACE *faces;		// aponta para as faces
    int total_faces;	// total de faces no objeto
};

// Definição dos vértices
VERT vertices[] =
{
    { -2, 0, -1 },	// 0 canto inf esquerdo tras.
    {  2, 0, -1 },	// 1 canfo inf direito  tras.
    {  2, 0,  1 },	// 2 canto inf direito  diant.
    { -2, 0,  1 },  // 3 canto inf esquerdo diant.
    { -2, 0.2, -1 },	// 4
    {  2, 0.2, -1 },	// 5
    {  2, 0.2,  1 },	// 6
    { -2, 0.2,  1 },    // 7
};

// Definição das faces
FACE faces[] =
{
    { 4, { 0,3,1,2 }},	// base
    { 4, { 4,7,5,6 }},
    { 4, { 4,0,7,3 }},
    { 4, { 7,3,6,2 }},
    { 4, { 6,2,5,1 }},
    { 4, { 4,0,5,1 }},
};

// Finalmente, define o objeto pirâmide
OBJ piramide =
{
    vertices, faces, 6
};

float rand_float(float a, float b)
{
    return ((b - a) * ((float)std::rand() / RAND_MAX)) + a;
}


class Disco
{
public:
    Disco(int peso, float r, float g, float b ) : peso(peso), xoffset(0), selected(false), r(r), g(g), b(b)
    {
        //
    }

    Disco(int peso) : peso(peso), xoffset(0), selected(false), r(0.f), g(0.f), b(0.f) {

        r = rand_float(0, 1.f);
        g = rand_float(0, 1.f);
        b = rand_float(0, 1.f);

    }


    int getPeso() const { return peso; }
    float getR() const { return r;}
    float getG() const { return g;}
    float getB() const { return b;}



    void setSelected(bool status) { selected = status;}
    bool getSelected() const { return selected; }

    //0 left, 1 right
    void move(float offset)
    {
        xoffset += offset;
    }

    void draw(int i)
    {
        glPushMatrix();

        float x = 0, y =  0, z = (0.21f * i);
        glTranslatef(x + xoffset, y, selected ? z = (1.7 - z) + z : z);


        glColor3f(r, g, b);
        glutWireTorus(0.1, 0.5f - (peso * 0.07f), 30, 500);

        glPopMatrix();
    }

private:
    int peso;
    float r, g, b;

    bool selected;

    float xoffset;

};

class Torre
{
public:
    Torre(float x, float y, float z, bool selected = false) : x(x), y(y), z(z), selected(selected) {};
    ~Torre()
    {
        while(!discos.empty())
        {
            Disco* top = discos.back();
            delete top;
            discos.pop_back();
        }

    }


    int getSize() const { return discos.size(); }

    void pushDisco(Disco* d)
    {
        discos.push_back(d);
    };
    void popDisco()
    {
        if(discos.empty())
            return;

        delete discos.back();
        discos.pop_back();
    };
    Disco* top() const
    {
        return discos.empty() ? nullptr : discos.back();
    }


    void draw()
    {

        glPushMatrix();

        glTranslatef(x, y, z);
        this->selected ? glColor3f(1.f, 0.f, 0.f) : glColor3f(0.2f, 0.0f, 0.9f);
        gluCylinder(qobj, 0.1, 0.1, 0.3 + (0.3 * MAX_DISK), 30, 70);


        glTranslatef(0.f, 0.f, 0.3f);
        for(unsigned int i = 0; i < discos.size(); i++)
        {
            Disco* d = discos.at(i);
            d->draw(i);
        }

        glPopMatrix();

    }

    bool getSelected() const
    {
        return selected;
    }
    void setSelected(bool status)
    {
        selected = status;
    }

    float getX() const
    {
        return x;
    }
    float getY() const
    {
        return y;
    }
    float getZ() const
    {
        return z;
    }

private:
    bool selected;
    float x, y, z;
    std::vector<Disco*> discos;



};




// Desenha um objeto em wireframe
void DesenhaObjetoWireframe(OBJ *objeto)
{

    // Percorre todas as faces
    for(int f=0; f < objeto->total_faces; f++)
    {
        glBegin(GL_QUAD_STRIP);
        // Percorre todos os vértices da face
        for(int v=0; v < objeto->faces[f].total; v++)
            glVertex3f(objeto->vertices[objeto->faces[f].ind[v]].x,
                       objeto->vertices[objeto->faces[f].ind[v]].y,
                       objeto->vertices[objeto->faces[f].ind[v]].z);
        glEnd();
    }
}



unsigned int tower_index = 0;
Torre* playing_tower = nullptr;

Torre* selected_tower = nullptr;
Disco* selected_disk = nullptr;

std::vector<Torre*> torres;


// Função callback de redesenho da janela de visualização
void Desenha(void)
{
    // Limpa a janela de visualização com a cor
    // de fundo definida previamente
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat luzDifusa[4]={0.7,0.7,0.7,1.0};
    GLfloat luzEspecular[4]={1.0, 1.0, 1.0, 1.0};
    GLfloat luzAmbiente[4]={0.2,0.2,0.2,1.0};
    GLfloat especularidade[4]={1.0,1.0,1.0,1.0};
    GLfloat posicaoLuz[4]={0.0, 50.0, 50.0, 1.0};
	GLint especMaterial = 60;
    GLfloat angle=45;

    glShadeModel(GL_SMOOTH);
    glMaterialfv(GL_FRONT,GL_SPECULAR, especularidade);
    glMateriali(GL_FRONT,GL_SHININESS,especMaterial);
    glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa );
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular );
	glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz );
	glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa );
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular );
	glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz );
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

    // Altera a cor do desenho para preto
    glColor3f(0.0f, 0.0f, 0.0f);

    //glutWireCube(30);
    //glutWireTeapot(30);
    DesenhaObjetoWireframe(&piramide);

    glRotatef(270,1.0f,0.0f,0.0f);


    for(unsigned int i = 0; i < torres.size(); i++)
    {
        Torre* t = torres.at(i);
        t->draw();
    }

    // Executa os comandos OpenGL
    glutSwapBuffers();
}

// Função usada para especificar a posição do observador virtual
void PosicionaObservador(void)
{
    // Especifica sistema de coordenadas do modelo
    glMatrixMode(GL_MODELVIEW);
    // Inicializa sistema de coordenadas do modelo
    glLoadIdentity();
    // Posiciona e orienta o observador
    glTranslatef(-obsX,-obsY,-obsZ);
    glRotatef(rotX,1,0,0);
    glRotatef(rotY,0,1,0);
}

// Função usada para especificar o volume de visualização
void EspecificaParametrosVisualizacao(void)
{
    // Especifica sistema de coordenadas de projeção
    glMatrixMode(GL_PROJECTION);
    // Inicializa sistema de coordenadas de projeção
    glLoadIdentity();

    // Especifica a projeção perspectiva(angulo,aspecto,zMin,zMax)
    gluPerspective(angle,fAspect,0.1,1200);

    PosicionaObservador();
}

// Função callback chamada quando o tamanho da janela é alterado
void AlteraTamanhoJanela(GLsizei w, GLsizei h)
{
    // Para previnir uma divisão por zero
    if ( h == 0 )
        h = 1;

    // Especifica as dimensões da viewport
    glViewport(0, 0, w, h);

    // Calcula a correção de aspecto
    fAspect = (GLfloat)w/(GLfloat)h;

    EspecificaParametrosVisualizacao();
}

// Função callback chamada para gerenciar eventos de teclas normais (ESC)
void Teclado (unsigned char tecla, int x, int y)
{
    if(tecla==27) // ESC ?
    {
        // Libera memória e finaliza programa
        exit(0);
    }
}

// > < /


// Função callback para tratar eventos de teclas especiais
void TeclasEspeciais (int tecla, int x, int y)
{
    switch (tecla)
    {
    case GLUT_KEY_HOME:
        if(angle>=10)
            angle -=5;
        break;
    case GLUT_KEY_END:
        if(angle<=150)
            angle +=5;
        break;

    case GLUT_KEY_LEFT:
        selected_tower->setSelected(false);

        tower_index == 0 ? tower_index = 2 : tower_index--;
        selected_tower = torres[tower_index];

        if(selected_disk != nullptr){
            tower_index != 2 ? selected_disk->move(-1.3f) : selected_disk->move(2.6f);
        }

        selected_tower->setSelected(true);

        break;
    case GLUT_KEY_RIGHT:
        selected_tower->setSelected(false);

        tower_index == 2 ? tower_index = 0 : tower_index++;
        selected_tower = torres[tower_index];

        selected_tower->setSelected(true);

        if(selected_disk != nullptr){
            tower_index == 0 ? selected_disk->move(-2.6f) : selected_disk->move(1.3f);
        }

        break;
    case GLUT_KEY_UP:
        if(selected_disk != nullptr || selected_tower->getSize() <= 0) break;

        playing_tower = selected_tower;
        selected_disk = selected_tower->top();
        selected_disk->setSelected(true);

        break;

    case GLUT_KEY_DOWN:

        if(selected_disk != nullptr){

            if(playing_tower == selected_tower){
                selected_disk->setSelected(false);
                selected_disk = nullptr;
            } else if(selected_tower->top() == nullptr || selected_tower->top()->getPeso() < selected_disk->getPeso()) {

                Disco* disco = new Disco(selected_disk->getPeso(), selected_disk->getR(), selected_disk->getG(), selected_disk->getB());
                playing_tower->popDisco();

                selected_tower->pushDisco(disco);
                selected_disk = nullptr;

                if(selected_tower->getSize() == MAX_DISK && tower_index == 2){
                    std::cout << "you win!" << std::endl;
                    PlaySound(TEXT("hino.wav"), NULL, SND_FILENAME | SND_ASYNC);
                }

            }
        }

        break;

    }
    EspecificaParametrosVisualizacao();
    glutPostRedisplay();
}



// Função callback para eventos de botões do mouse
void GerenciaMouse(int button, int state, int x, int y)
{
    if(state==GLUT_DOWN)
    {
        // Salva os parâmetros atuais
        x_ini = x;
        y_ini = y;
        obsX_ini = obsX;
        obsY_ini = obsY;
        obsZ_ini = obsZ;
        rotX_ini = rotX;
        rotY_ini = rotY;
        bot = button;
    }
    else
        bot = -1;
}

// Função callback para eventos de movimento do mouse
#define SENS_ROT	10.0
#define SENS_OBS	15.0
#define SENS_TRANSL	30.0
void GerenciaMovim(int x, int y)
{
    // Botão esquerdo ?
    if(bot==GLUT_LEFT_BUTTON)
    {
        // Calcula diferenças
        int deltax = x_ini - x;
        int deltay = y_ini - y;
        // E modifica ângulos
        rotY = rotY_ini - deltax/SENS_ROT;
        rotX = rotX_ini - deltay/SENS_ROT;
    }
    // Botão direito ?
    else if(bot==GLUT_RIGHT_BUTTON)
    {
        // Calcula diferença
        int deltaz = y_ini - y;
        // E modifica distância do observador
        obsZ = obsZ_ini + deltaz/SENS_OBS;
    }
    // Botão do meio ?
    else if(bot==GLUT_MIDDLE_BUTTON)
    {
        // Calcula diferenças
        int deltax = x_ini - x;
        int deltay = y_ini - y;
        // E modifica posições
        obsX = obsX_ini + deltax/SENS_TRANSL;
        obsY = obsY_ini - deltay/SENS_TRANSL;
    }
    PosicionaObservador();
    glutPostRedisplay();
}

// Função responsável por inicializar parâmetros e variáveis
void Inicializa (void)
{
    // Define a cor de fundo da janela de visualização como branca
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Inicializa a variável que especifica o ângulo da projeção
    // perspectiva
    angle=15;

    // Inicializa as variáveis usadas para alterar a posição do
    // observador virtual
    rotX = 0;
    rotY = 0;
    obsX = 0;
    obsY = 0.5;
    obsZ = 20;

    glEnable(GL_DEPTH_TEST);

    qobj = gluNewQuadric();
    gluQuadricNormals(qobj, GLU_SMOOTH);

    //inicializa torres
    Torre* t = new Torre(-1.3f, 0.2f, 0.f, true);

    for(unsigned int i = 0; i < MAX_DISK; i++){
        t->pushDisco(new Disco(i));
    }

    torres.push_back(t);
    torres.push_back(new Torre(0.f, 0.2f, 0.f));
    torres.push_back(new Torre(1.3f, 0.2f, 0.f));

    selected_tower = t; //primeira torre


}


// Programa Principal
int main(void)
{
    std::srand(std::time(nullptr));


    std::cout << "Digite a quantidade de discos (max 6): ";
    std::cin >> MAX_DISK;

    if(MAX_DISK > 6) MAX_DISK = 6;


    // Define do modo de operação da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Especifica a posição inicial da janela GLUT
    glutInitWindowPosition(5,5);

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(650,650);

    // Cria a janela passando como argumento o título da mesma
    glutCreateWindow("O Igor é corno conformado!");

    // Registra a função callback de redesenho da janela de visualização
    glutDisplayFunc(Desenha);

    // Registra a função callback de redimensionamento da janela de visualização
    glutReshapeFunc(AlteraTamanhoJanela);

    // Registra a função callback para eventos de botões do mouse
    glutMouseFunc(GerenciaMouse);

    // Registra a função callback para eventos de movimento do mouse
    glutMotionFunc(GerenciaMovim);

    // Registra a função callback para tratamento das teclas normais
    glutKeyboardFunc (Teclado);

    // Registra a função callback para tratamento das teclas especiais
    glutSpecialFunc (TeclasEspeciais);

    // Chama a função responsável por fazer as inicializações
    Inicializa();

    // Inicia o processamento e aguarda interações do usuário
    glutMainLoop();

    for(unsigned int i = 0; i < torres.size(); i++)
    {
        Torre* t = torres.back();
        delete t;
        torres.pop_back();
    }

    return 0;
}
