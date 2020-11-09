#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;
bool TextureChange = true;

int Alpha = 0;

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 'Q')
	{
		TextureChange = !TextureChange;
	}

	if (key == 'W')
	{
		Alpha++;
		if (Alpha == 3)
			Alpha = 0;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId[2];

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId[0]);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId[0]);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	OpenGL::LoadBMP("texture2.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//���������� �� ��� ��������
	glGenTextures(1, &texId[1]);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId[1]);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}


double* not_Normal(double A[3], double B[3], double C[3])
{
	double vector1[] = { B[0] - A[0], B[1] - A[1] ,B[2] - A[2] };

	double vector2[] = { C[0] - A[0], C[1] - A[1],C[2] - A[2] };

	double vector_normali[] = { vector1[1] * vector2[2] - vector2[1] * vector1[2], -vector1[0] * vector2[2] + vector2[0] * vector1[2], vector1[0] * vector2[1] - vector2[0] * vector1[1] };

	double lenght = sqrt(vector_normali[0] * vector_normali[0] + vector_normali[1] * vector_normali[1] + vector_normali[2] * vector_normali[2]);

	vector_normali[0] /= lenght;
	vector_normali[1] /= lenght;
	vector_normali[2] /= lenght;

	return vector_normali;
}


double * Normal (double A[3], double B[3], double C[3]) 
{
	double vector1[] = { B[0] - A[0], B[1] - A[1] ,B[2] - A[2] };

	double vector2[] = { C[0] - A[0], C[1] - A[1],C[2] - A[2] };

	double vector_normali[] = { vector1[1] * vector2[2] - vector2[1] * vector1[2], -vector1[0] * vector2[2] + vector2[0] * vector1[2], vector1[0] * vector2[1] - vector2[0] * vector1[1] };

	double lenght = -sqrt(vector_normali[0] * vector_normali[0] + vector_normali[1] * vector_normali[1] + vector_normali[2] * vector_normali[2]);

	vector_normali[0] /= lenght;
	vector_normali[1] /= lenght;
	vector_normali[2] /= lenght;

	return vector_normali;
}

double NewCoordX(double OldCoord) 
{
	return OldCoord / 12;
}

double NewCoordY(double OldCoord) 
{
	return (OldCoord + 6) / 10;
}


void vipuklost_calc(std::vector<double> & x, std::vector<double> & y) 
{
	const double x0 = 5;
	const double y0 = -4;
	const double r = 4;
	const double b = -2;

	x.push_back(9);
	y.push_back(-4);

	for (double i = 0; i <= 3.15; i += 0.001)
	{
		x.push_back(x0 + r * cos(i));
		y.push_back(y0 + b * sin(i) * sin(i));
	}

	x.push_back(1);
	y.push_back(-4);

}

void vipuclost_draw_top(std::vector<double> x, std::vector<double> y)
{
	glBegin(GL_TRIANGLES);

	double A[] = { 5, -4, 5 };


	for (int i = 0; i < x.size() - 1; i++)
	{
		double B[] = { x[i], y[i], 5 };
		double C[] = { x[i + 1], y[i + 1], 5 };

		glNormal3dv(Normal(A, B, C));

		glTexCoord2d(NewCoordX(A[0]), NewCoordY(A[1]));
		glVertex3dv(A);

		glTexCoord2d(NewCoordX(B[0]), NewCoordY(B[1]));
		glVertex3dv(B);

		glTexCoord2d(NewCoordX(C[0]), NewCoordY(C[1]));
		glVertex3dv(C);
	}
	
	glEnd();
}

void vpuklost_calc(std::vector<double>& x, std::vector<double>& y)
{
	const double x0 = 9;
	const double y0 = 0;
	const double a = 3;
	const double b = -1;

	x.push_back(12);
	y.push_back(0);

	//* ������ 
	for (double i = 0; i <= 3.15; i += 0.001)
	{
		x.push_back(x0 + a * cos(i));
		y.push_back(y0 + b * sin(i) * sin(i));
	}
	//*/


	x.push_back(6);
	y.push_back(0);

}

void vpuclost_draw_top(std::vector<double> x, std::vector<double> y)
{
	glBegin(GL_TRIANGLES);

	double A[] = { 9, -4, 5 };

	for (int i = 0; i < x.size() - 1; i++)
	{
		double B[] = { x[i], y[i], 5 };
		double C[] = { x[i + 1], y[i + 1], 5 };


		glNormal3dv(not_Normal(A, B, C));
		

		glTexCoord2d(NewCoordX(A[0]), NewCoordY(A[1]));
		glVertex3dv(A);

		glTexCoord2d(NewCoordX(B[0]), NewCoordY(B[1]));
		glVertex3dv(B);

		glTexCoord2d(NewCoordX(C[0]), NewCoordY(C[1]));
		glVertex3dv(C);
	}

	glEnd();
}

void figure (int flag)
{
	void steni();
	void osnova();
	void vipuklost(std::vector<double> x, std::vector<double> y);
	void vpuklost(std::vector<double> x, std::vector<double> y);
	void osnova2();

	std::vector<double> x_vip;
	std::vector<double> y_vip;

	std::vector<double> x_vp;
	std::vector<double> y_vp;

	vipuklost_calc(x_vip, y_vip); //������� ��������� ����������
	vpuklost_calc(x_vp, y_vp);    //������� ��������� ���������

	glColor3d(0.5, 0.5, 0.5);
	osnova();					  //��������� ���� ������
	
	glColor3d(0.2, 0.2, 0.2);
	steni();					  //��������� ������
	
	glColor3d(0.3, 0.3, 0.3);
	vipuklost(x_vip, y_vip);	  //��������� ����� � ���� ����������
	
	glColor3d(0.4, 0.4, 0.4);
	vpuklost(x_vp, y_vp);		  //��������� ����� � ���� ���������

	if (flag == 1) 
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	if (flag == 2) 
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4d(0.7, 0.1, 0.1, 0.3);
	}
	

	osnova2();						  //��������� ����� ������
	vipuclost_draw_top(x_vip, y_vip); //��������� ����� ����������
	vpuclost_draw_top(x_vp, y_vp);	  //��������� ����� ���������


	if (flag != 0)
		glDisable(GL_BLEND);
	
}

void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);
;

	//��������� ���������
	GLfloat amb[] = { 0.5, 0.3, 0, 0.5 };
	GLfloat dif[] = { 0.9, 0.2, 0.1, 0.6 };
	GLfloat spec[] = { 0.9, 0.1, 0.1, 0.9 };
	GLfloat sh = 0.7f * 256;

	/*// �������� �� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;
	*/
	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
	//������ �����
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  

	if (TextureChange)
		glBindTexture(GL_TEXTURE_2D, texId[0]);
	else 
		glBindTexture(GL_TEXTURE_2D, texId[1]);

	figure(Alpha);


   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "Q - ������� ��������" << std::endl;
	ss << "W - ������� ��� ������������" << std::endl;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;


	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

void vpuklost(std::vector<double> x, std::vector<double> y)
{
	double* colour = new double[] { 0.1, 0.5, 1 };

	double inst_coord = 0;
	double incr = 1. / x.size() / 2;

	
	for (double n = 0; n < 1; n += 5)
	{
		glBegin(GL_TRIANGLES);

		double A[] = { 9, -4, n };

		for (int i = 0; i < x.size() - 1; i++)
		{
			double B[] = { x[i], y[i], n };
			double C[] = { x[i + 1], y[i + 1], n };


			if (n == 5)
				glNormal3dv(not_Normal(A, B, C));
			else
				glNormal3dv(Normal(A, B, C));


			glTexCoord2d(NewCoordX(A[0]), NewCoordY(A[1]));
			glVertex3dv(A);

			glTexCoord2d(NewCoordX(B[0]), NewCoordY(B[1]));
			glVertex3dv(B);

			glTexCoord2d(NewCoordX(C[0]), NewCoordY(C[1]));
			glVertex3dv(C);
		}

		glEnd();

	}

	glBegin(GL_QUADS);
	for (double i = 0; i < x.size() - 1; i++)
	{
		double A[] = { x[i], y[i], 0 };
		double B[] = { x[i + 1], y[i + 1], 0 };
		double C[] = { x[i], y[i], 5 };
		double D[] = { x[i + 1], y[i + 1], 5 };


		glNormal3dv(Normal(B, A, D));

		glTexCoord2d(i / (x.size() - 1), 0);
		glVertex3dv(A);
		glTexCoord2d(i / (x.size() - 1), 0);
		glVertex3dv(B);
		glTexCoord2d(i / (x.size() - 1), 1);
		glVertex3dv(D);
		glTexCoord2d(i / (x.size() - 1), 1);
		glVertex3dv(C);

		
	}
	glEnd();

	delete[] colour;
}

void vipuklost(std::vector<double> x, std::vector<double> y)
{
	
	double* colour = new double[] { 0.1, 0.5, 1 };

	double inst_coord = 0;
	double incr = 1. / x.size() / 2;

	
	for (double n = 0; n < 1; n += 5)
	{
		glBegin(GL_TRIANGLES);

		double A[] = { 5, -4, n };

		

		for (int i = 0; i < x.size() - 1; i++)
		{
			double B[] = { x[i], y[i], n };
			double C[] = { x[i + 1], y[i + 1], n };


			if (n == 5)
				glNormal3dv(Normal(A, B, C));
			else
				glNormal3dv(not_Normal(A, B, C));


			glTexCoord2d(NewCoordX(A[0]), NewCoordY(A[1]));
			glVertex3dv(A);

			glTexCoord2d(NewCoordX(B[0]), NewCoordY(B[1]));
			glVertex3dv(B);

			glTexCoord2d(NewCoordX(C[0]), NewCoordY(C[1]));
			glVertex3dv(C);
		}
		glEnd();
	}
	


	glBegin(GL_QUADS);
	for (double i = 1; i < x.size() - 2; i++)
	{
		double A[] = { x[i], y[i], 0 };
		double B[] = { x[i + 1], y[i + 1], 0 };
		double C[] = { x[i], y[i], 5 };
		double D[] = { x[i + 1], y[i + 1], 5 };

		glNormal3dv(not_Normal(B, A, D));

		glTexCoord2d(i / (x.size()-2), 0);
		glVertex3dv(A);
		glTexCoord2d(i / (x.size() - 2), 0);
		glVertex3dv(B);
		glTexCoord2d(i / (x.size() - 2), 1);
		glVertex3dv(D);
		glTexCoord2d(i / (x.size() - 2), 1);
		glVertex3dv(C);
	}
	glEnd();

	delete[] colour;
}

void steni()
{
	glBegin(GL_QUADS);
	//
	glColor3d(0.3, 0.3, 0.3);

	double A[] = { 0, 0, 0 };
	double B[] = { 2, 0, 0 };
	double AS[] = { 0, 0, 5 };
	double BS[] = { 2, 0, 5 };

	glNormal3dv(Normal(A, B, BS));

	glTexCoord2d(1, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 0);
	glVertex3dv(B);
	glTexCoord2d(0, 1);
	glVertex3dv(BS);
	glTexCoord2d(1, 1);
	glVertex3dv(AS);
	//
	glColor3d(0.4, 0.4, 0.4);

	double A1[] = { 2, 0, 0 };
	double B1[] = { 4, 3, 0 };
	double AS1[] = { 2, 0, 5 };
	double BS1[] = { 4, 3, 5 };

	glNormal3dv(Normal(A1, B1, BS1));

	glTexCoord2d(1, 0);
	glVertex3dv(A1);
	glTexCoord2d(0, 0);
	glVertex3dv(B1);
	glTexCoord2d(0, 1);
	glVertex3dv(BS1);
	glTexCoord2d(1, 1);
	glVertex3dv(AS1);
	//
	glColor3d(0.5, 0.5, 0.5);

	double A2[] = { 4, 3, 0 };
	double B2[] = { 7, 3, 0 };
	double AS2[] = { 4, 3, 5 };
	double BS2[] = { 7, 3, 5 };

	glNormal3dv(Normal(A2, B2, BS2));

	glTexCoord2d(1, 0);
	glVertex3dv(A2);
	glTexCoord2d(0, 0);
	glVertex3dv(B2);
	glTexCoord2d(0, 1);
	glVertex3dv(BS2);
	glTexCoord2d(1, 1);
	glVertex3dv(AS2);
	//

	double A3[] = { 7, 3, 0 };
	double B3[] = { 6, 0, 0 };
	double AS3[] = { 7, 3, 5 };
	double BS3[] = { 6, 0, 5 };

	glNormal3dv(Normal(A3, B3, BS3));

	glTexCoord2d(1, 0);
	glVertex3dv(A3);
	glTexCoord2d(0, 0);
	glVertex3dv(B3);
	glTexCoord2d(0, 1);
	glVertex3dv(BS3);
	glTexCoord2d(1, 1);
	glVertex3dv(AS3);
	//

	double A5[] = { 12, 0, 0 };
	double B5[] = { 9, -4, 0 };
	double AS5[] = { 12, 0, 5 };
	double BS5[] = { 9, -4, 5 };

	glNormal3dv(Normal(A5, B5, BS5));

	glTexCoord2d(1, 0);
	glVertex3dv(A5);
	glTexCoord2d(0, 0);
	glVertex3dv(B5);
	glTexCoord2d(0, 1);
	glVertex3dv(BS5);
	glTexCoord2d(1, 1);
	glVertex3dv(AS5);
	//

	double A7[] = { 1, -4, 0 };
	double B7[] = { 0, 0, 0 };
	double AS7[] = { 1, -4, 5 };
	double BS7[] = { 0, 0, 5 };

	glNormal3dv(Normal(A7, B7, BS7));

	glTexCoord2d(1, 0);
	glVertex3dv(A7);
	glTexCoord2d(0, 0);
	glVertex3dv(B7);
	glTexCoord2d(0, 1);
	glVertex3dv(BS7);
	glTexCoord2d(1, 1);
	glVertex3dv(AS7);

	glEnd();
}

void osnova()
{
	glBegin(GL_QUADS);

	double A[] = { 0, 0, 0 };
	double B[] = { 6, 0, 0 };
	double C[] = { 9, -4, 0 };
	double D[] = { 1, -4, 0 };


	glNormal3dv(Normal(B, A, C));

	glTexCoord2d(NewCoordX(A[0]), NewCoordY(A[1]));
	glVertex3dv(A);
	glTexCoord2d(NewCoordX(B[0]), NewCoordY(B[1]));
	glVertex3dv(B);
	glTexCoord2d(NewCoordX(C[0]), NewCoordY(C[1]));
	glVertex3dv(C);
	glTexCoord2d(NewCoordX(D[0]), NewCoordY(D[1]));
	glVertex3dv(D);

	//
	double A1[] = { 2, 0, 0 };
	double B1[] = { 6, 0, 0 };
	double C1[] = { 7, 3, 0 };
	double D1[] = { 4, 3, 0 };

	glNormal3dv(not_Normal(B1, A1, C1));

	glTexCoord2d(NewCoordX(A1[0]), NewCoordY(A1[1]));
	glVertex3dv(A1);
	glTexCoord2d(NewCoordX(B1[0]), NewCoordY(B1[1]));
	glVertex3dv(B1);
	glTexCoord2d(NewCoordX(C1[0]), NewCoordY(C1[1]));
	glVertex3dv(C1);
	glTexCoord2d(NewCoordX(D1[0]), NewCoordY(D1[1]));
	glVertex3dv(D1);

	glEnd();

}

void osnova2()
{
	glBegin(GL_QUADS);

	double A[] = { 0, 0, 5 };
	double B[] = { 6, 0, 5 };
	double C[] = { 9, -4, 5 };
	double D[] = { 1, -4, 5 };

	
	glNormal3dv(not_Normal(B, A, C));

	glTexCoord2d(NewCoordX(A[0]), NewCoordY(A[1]));
	glVertex3dv(A);
	glTexCoord2d(NewCoordX(B[0]), NewCoordY(B[1]));
	glVertex3dv(B);
	glTexCoord2d(NewCoordX(C[0]), NewCoordY(C[1]));
	glVertex3dv(C);
	glTexCoord2d(NewCoordX(D[0]), NewCoordY(D[1]));
	glVertex3dv(D);

	//
	double A1[] = { 2, 0, 5 };
	double B1[] = { 6, 0, 5 };
	double C1[] = { 7, 3, 5 };
	double D1[] = { 4, 3, 5 };



	glNormal3dv(Normal(B1, A1, C1));

	glTexCoord2d(NewCoordX(A1[0]), NewCoordY(A1[1]));
	glVertex3dv(A1);
	glTexCoord2d(NewCoordX(B1[0]), NewCoordY(B1[1]));
	glVertex3dv(B1);
	glTexCoord2d(NewCoordX(C1[0]), NewCoordY(C1[1]));
	glVertex3dv(C1);
	glTexCoord2d(NewCoordX(D1[0]), NewCoordY(D1[1]));
	glVertex3dv(D1);

	glEnd();

}

