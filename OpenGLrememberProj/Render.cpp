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

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
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

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId[0]);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId[0]);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	OpenGL::LoadBMP("texture2.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId[1]);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId[1]);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

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

	//* эллипс 
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

	vipuklost_calc(x_vip, y_vip); //рассчет координат выпуклости
	vpuklost_calc(x_vp, y_vp);    //рассчет координат впуклости

	glColor3d(0.5, 0.5, 0.5);
	osnova();					  //отрисовка низа фигуры
	
	glColor3d(0.2, 0.2, 0.2);
	steni();					  //отрисовка граней
	
	glColor3d(0.3, 0.3, 0.3);
	vipuklost(x_vip, y_vip);	  //отрисовка грани и низа выпуклости
	
	glColor3d(0.4, 0.4, 0.4);
	vpuklost(x_vp, y_vp);		  //отрисовка грани и низа впуклости

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
	

	osnova2();						  //отрисовка верха фигуры
	vipuclost_draw_top(x_vip, y_vip); //отрисовка верха выпуклости
	vpuclost_draw_top(x_vp, y_vp);	  //отрисовка верха впуклости


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

	//настройка материала
	GLfloat amb[] = { 0.5, 0.3, 0, 0.5 };
	GLfloat dif[] = { 0.9, 0.2, 0.1, 0.6 };
	GLfloat spec[] = { 0.9, 0.1, 0.1, 0.9 };
	GLfloat sh = 0.7f * 256;

	/*// значения по умолчанию
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;
	*/
	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	if (TextureChange)
		glBindTexture(GL_TEXTURE_2D, texId[0]);
	else 
		glBindTexture(GL_TEXTURE_2D, texId[1]);

	figure(Alpha);


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "Q - сменить текстуру" << std::endl;
	ss << "W - сменить вид прозрачности" << std::endl;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;


	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
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

