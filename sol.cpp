#include <iostream>
#include <fstream>
#include <stdio.h>

#include <SOIL/SOIL.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;

float matShine[] = { 4.0 };                     // expoente especular (shininess)
static float larguraJanela, alturaJanela;       // (w,h) da janela
static bool isLightingOn = false;               // O sistema de iluminação está ligado?
static float anguloEsferaY = 0;                 // Rotação das esferas em torno do eixo y
static int esferaLados = 200;                   // Quantas subdivisões latitudinais/longitudinais da esfera
static int textures[10];
int camera = 1;

GLuint carregaTextura(const char* arquivo) {
    GLuint idTextura = SOIL_load_OGL_texture(
                           arquivo,
                           SOIL_LOAD_AUTO,
                           SOIL_CREATE_NEW_ID,
                           SOIL_FLAG_INVERT_Y
                       );

    if (idTextura == 0) {
        printf("Erro do SOIL: '%s'\n", SOIL_last_result());
    }

    return idTextura;
}

// Configuração inicial do OpenGL e GLUT
void setup(void)
{
    //glClearColor(.4,.4,.4, 0.0);                    // fundo cinza
    glClearColor(0,0,0, 0.0);
    glEnable(GL_DEPTH_TEST);                        // Ativa teste Z

    // Propriedades do material da esfera
    float matAmbAndDif[] = {1.0, 1.0, 1.0, 1.0};    // cor ambiente e difusa: branca
    float matSpec[] = { 1.0, 1.0, 1,0, 1.0 };       // cor especular: branca

    // Definindo as propriedades do material
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShine);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carrega a textura
    FILE* fp;
	fp = fopen("textures.txt", "r");
	if(fp == NULL){
		printf("Nao foi possivel abrir o arquivo!\n");
		exit(1);
	}
	rewind(fp);
	char textura[20];
    for(int i=0; i<10; i++){
        fscanf(fp, " %s", textura);
        textures[i] = carregaTextura(textura);
        if(textures[i] == 0)
        {
            cout << "Problema ao carregar textura: " << SOIL_last_result() << endl;
        }
    }
    
    // Não mostrar faces do lado de dentro
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
}

// Desenha uma esfera na origem, com certo raio e subdivisões
// latitudinais e longitudinais
//
// Não podemos usar glutSolidSphere, porque ela não chama
// glTexCoord para os vértices. Logo, se você quer texturizar
// uma esfera, não pode usar glutSolidSphere
void solidSphere(int radius, int stacks, int columns)
{
    // cria uma quádrica
    GLUquadric* quadObj = gluNewQuadric();
    // estilo preenchido... poderia ser GLU_LINE, GLU_SILHOUETTE
    // ou GLU_POINT
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    // chama 01 glNormal para cada vértice.. poderia ser
    // GLU_FLAT (01 por face) ou GLU_NONE
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    // chama 01 glTexCoord por vértice
    gluQuadricTexture(quadObj, GL_TRUE);
    // cria os vértices de uma esfera
    gluSphere(quadObj, radius, stacks, columns);
    // limpa as variáveis que a GLU usou para criar
    // a esfera
    gluDeleteQuadric(quadObj);
}

// Callback de desenho
void desenhaCena()
{
    // Propriedades das fontes de luz
    float lightAmb[] = { 0.0, 0.0, 0.0, 1.0 };
    float lightDif[] = { 1.0, 1.0, 1.0, 1.0 };
    float lightSpec[] = { 1.0, 1.0, 1.0, 1.0 };
    float lightPos[] = { 0.0, 0.0, 0.0, 1 };
    float globAmb[] = { 0.30, 0.30, 0.30, 1.0 };

    // Propriedades da fonte de luz LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb);        // Luz ambiente global

    //propriedades das esferas (posições, raio)
    float esfera[9][4] = {{0.0, 0.0, 0.0, 5.0}, {0.0, 7.0, -2.0, 1.0}, {0.0, 10.5, 2.0, 1.5}, {0.0, 14.0, -1.5, 1.5}, {0.0, 17.5, -1.5, 1.0}, {0.0, 22.5, 2.0, 3.0}, {0.0, 29.5, 0.0, 3.0}, {0.0, 36.5, -2.0, 2.0}, {0.0, 41.5, -1.0, 2.0}};

    // Ativa a fonte de luz
    glEnable(GL_LIGHT0);

    // Limpa a tela e o z-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    // Posiciona a câmera
    switch(camera){
        case 1: gluLookAt(0, 0, 60,
                          0, 0, 0,
                          0, 1, 0);
                break;
        case 2: gluLookAt(0, 60, 0,
                          0, 0, 0,
                          1, 0, 0);
                break;
    }

    // Desabilita iluminação para desenhar a esfera que representa a luz e o fundo
    glDisable(GL_LIGHTING);

    // Light0 e esfera indicativa
    glPushMatrix();
    	glRotatef(0.0, 1.0, 0.0, 0.0); // Rotação no eixo x
        glRotatef(anguloEsferaY, 0.0, 1.0, 0.0); // Rotação no eixo y
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glTranslatef(lightPos[0], lightPos[1], lightPos[2]);
        glColor3f(1.0, 1.0, 1.0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        solidSphere(esfera[0][3], esferaLados, esferaLados);
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, textures[9]);
    glPushMatrix();
        solidSphere(60, esferaLados, esferaLados);
        // glutSolidSphere(1.5, esferaLados, esferaLados);
        // glutSolidCube(1.5);
        // glutSolidTeapot(1.5);
    glPopMatrix();

    if (isLightingOn) {
        glEnable(GL_LIGHTING);
    }

    // Desenha as esferas grandes e bem arredondadas
    for(int i=1; i<9; i++){
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glPushMatrix();
            glRotatef(anguloEsferaY*(10-i), 0.0, 1.0, 0.0);
            glRotatef(-90, 1.0, 0.0, 0.0);
            glTranslatef(esfera[i][0], esfera[i][1], esfera[i][2]);
            glRotatef(anguloEsferaY, 0.0, 0.0, 1.0);
            solidSphere(esfera[i][3], esferaLados, esferaLados);
            // glutSolidSphere(1.5, esferaLados, esferaLados);
            // glutSolidCube(1.5);
            // glutSolidTeapot(1.5);
        glPopMatrix();
    }
    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
}

// Callback do evento de teclado
void keyInput(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1': 
        camera = 1;
        break;
    case '2':
        camera = 2;
        break;
    case 27:
        exit(0);
        break;
    case 'l':
    case 'L':
        isLightingOn = !isLightingOn;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

// Callback de redimensionamento
void resize(int w, int h)
{
    larguraJanela = w;
    alturaJanela = h;

    glViewport (0, 0, w, h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w/(float)h, 1.0, 120.0);
    glMatrixMode(GL_MODELVIEW);
}

void rotacionaEsfera() {
    anguloEsferaY += .1f;
    glutPostRedisplay();
}

// Imprime a ajuda no console
void imprimirAjudaConsole(void)
{
    cout << "Ajuda:" << endl;
    cout << "  Aperte 'l' para ligar/desligar a iluminacao do OpenGL." << endl
         << "  Aperte '1/2' para alterar a câmera." << endl;
}

int main(int argc, char *argv[])
{
    imprimirAjudaConsole();
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition (250, 100);
    glutCreateWindow("Sistema Solar");
    glutDisplayFunc(desenhaCena);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glutIdleFunc(rotacionaEsfera);

    setup();

    glutMainLoop();
}
