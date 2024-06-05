#include <GL/glut.h>
#include <glm/glm.hpp>
#include <SOIL/SOIL.h>
#include <iostream>
#include <numbers>
#include <cmath>

constexpr int RESOLUTION = 100;
float LightAmbient[] = {0.9f, 0.9f, 0.9f, 3.0f};
float LightDiffuse[] = {0.9f, 0.9f, 0.9f, 3.0f};
float LightPosition[] = {1.0f, 1.0f, 0.5f, 0.0f};
bool wire_frame = false, normals = false;
int texture;

int xold, yold, left_click, right_click;
float xrot = 30, yrot = 15, ztrans = 5;

float surface[6 * RESOLUTION * (RESOLUTION + 1)];
float normal[6 * RESOLUTION * (RESOLUTION + 1)];
constexpr float amplitude[2] = {0.01, 0.03};
constexpr float wavelength[2] = {0.3, 0.5};
constexpr float speed[2] = {-0.2, -0.3};
constexpr glm::vec2 center[2] {
    glm::vec2(),
    glm::vec2(-0.4, -0.4)
};
constexpr float frequency[2] = {
    2.f * std::numbers::pi / wavelength[0],
    2.f * std::numbers::pi / wavelength[1],
};
constexpr float phi[2] = {
    speed[0] * frequency[0],
    speed[1] * frequency[1]
};

void changeSize(int w, int h)
{
    if(h == 0) h = 1;
    float ratio = 1.0 * w / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45, ratio, 0.1, 15);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

void processNormalKeys(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27:
        exit(0);
        break;
    case 'l':
        wire_frame = !wire_frame;
        break;
    case 'n':
        normals = !normals;
        break;
    }
}

void processMouse(int button, int state, int x, int y)
{
    if(GLUT_LEFT_BUTTON == button) {
        left_click = state;
    } else if(GLUT_RIGHT_BUTTON == button) {
        right_click = state;
    }
    xold = x;
    yold = y;
}

void mouseMotion(int x, int y)
{
    if(left_click == GLUT_DOWN) {
        xrot += (x - xold) / 5.f;
        yrot += (y - yold) / 5.f;
        if(xrot > 90) xrot = 90;
        if(xrot < -90) xrot = -90;
    } else if(right_click == GLUT_DOWN) {
        ztrans += (y - yold) / 50.f;
        if(ztrans < 0.5) ztrans = 0.5;
        if(ztrans > 10) ztrans = 10;
    }
    glutPostRedisplay();
    xold = x;
    yold = y;
}

void LoadGLTextures()
{
    texture = SOIL_load_OGL_texture("../reflection.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if(texture == 0) {
        std::cout << "Error loading texture\n";
        exit(1);
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
}

void initGL()
{
    LoadGLTextures();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);

    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

float tht(int i, float x, float y)
{
    return glm::length(glm::vec2(x - center[i].x, y - center[i].y));
}

glm::vec2 dWave(int i, float x, float y, float t)
{
    float theta = tht(i, x, y);
    glm::vec2 A = amplitude[i] * frequency[i] * glm::vec2(x, y) / theta;
    return A * cosf(theta * frequency[i] + phi[i] * t);
}

float waveHeight(float x, float y, float t)
{
    float height{};
    for(int i = 0; i < 2; ++i) {
        float theta = tht(i, x, y);
        height += amplitude[i] * sinf(theta * frequency[i] + phi[i] * t);
    }
    return height;
}

glm::vec3 waveNormal(float x, float y, float t)
{
    glm::vec2 dW{};
    for(int i = 0; i < 2; ++i) {
        dW += dWave(i, x, y, t);
    }
    glm::vec3 n {-dW.x, 1.f, -dW.y};
    if(glm::length(n) > 1e-5f) {
        n = glm::normalize(n);
    } else n = glm::vec3(0, 1, 0);
    return n;
}

void renderScene()
{
	const float t = glutGet (GLUT_ELAPSED_TIME) / 1000.;
	const float delta = 2. / RESOLUTION;
	const unsigned int length = 2 * (RESOLUTION + 1);
	const float xn = (RESOLUTION + 1) * delta + 1;
	unsigned int i;
	unsigned int j;
	float x;
	float y;
	unsigned int indice;
	unsigned int preindice;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -ztrans);
	glRotatef(yrot, 1, 0, 0);
	glRotatef(xrot, 0, 1, 0);

	/* Vertices */
	for (j = 0; j < RESOLUTION; j++)
	{
		y = (j + 1) * delta - 1;
		for (i = 0; i <= RESOLUTION; i++)
		{
			indice = 6 * (i + j * (RESOLUTION + 1));

			x = i * delta - 1;
			surface[indice + 3] = x;
			surface[indice + 4] = waveHeight(x, y, t);
			surface[indice + 5] = y;
			if (j != 0)
			{
				/* Values were computed during the previous loop */
				preindice = 6 * (i + (j - 1) * (RESOLUTION + 1));
				surface[indice] = surface[preindice + 3];
				surface[indice + 1] = surface[preindice + 4];
				surface[indice + 2] = surface[preindice + 5];
			}
			else
			{
				surface[indice] = x;
				surface[indice + 1] = waveHeight(x, -1, t);
				surface[indice + 2] = -1;
			}
			/* Normals */
			glm::vec3 n = waveNormal(surface[indice],surface[indice + 2],t);
			normal[indice] = n.x;
			normal[indice+1] = n.y;
			normal[indice+2] = n.z;	

			n = waveNormal(surface[indice + 3],surface[indice + 5],t);
			normal[indice +3] = n.x;
			normal[indice+4] = n.y;
			normal[indice+5] = n.z;	
		}
	}

	/* The ground */
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glColor4f (1.0f, 0.9f, 0.7f,1.0f);
	glBegin (GL_QUADS);
		glNormal3f(0.0f,1.0f,0.0f);
		glVertex3f (-1.0f, 0.0f, -1.0f);
		glVertex3f (-1.0f, 0.0f, 1.0f);
		glVertex3f ( 1.0f, 0.0f, 1.0f);
		glVertex3f ( 1.0f, 0.0f, -1.0f);	
	glEnd();

	glTranslatef (0, 0.2, 0);

	/* Render wireframe? */
	if (wire_frame != 0)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	/* The water */
	glEnable(GL_TEXTURE_2D);
	glColor4f(0.0f, 0.5f, 1.0f, 1.0f);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glNormalPointer(GL_FLOAT, 0, normal);
	glVertexPointer(3, GL_FLOAT, 0, surface);
	for (i = 0; i < RESOLUTION; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, i * length, length);

	/* Draw normals? */
	if (normals != 0)
	{
		glDisable (GL_TEXTURE_2D);
		glColor3f (1, 0, 0);
		glBegin (GL_LINES);
			for (j = 0; j < RESOLUTION; j++)
				for (i = 0; i <= RESOLUTION; i++)
				{
					indice = 6 * (i + j * (RESOLUTION + 1));
					glVertex3fv (&(surface[indice]));
					glVertex3f (surface[indice] + normal[indice] / 50,
						surface[indice + 1] + normal[indice + 1] / 50,
						surface[indice + 2] + normal[indice + 2] / 50);
				}
		glEnd ();
	}

	/* End */
	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Water Simulation");

    initGL();

    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processNormalKeys);
    glutMouseFunc(processMouse);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}