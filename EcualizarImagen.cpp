#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cassert>
#include <iostream> //Debug
#include "EcualizarImagen.hpp"
#include "Histograma.hpp"
#include "HistogramaAcumulado.hpp"

using namespace cv;
using namespace std; //Debug

namespace FSIV
{
  EcualizarImagen::EcualizarImagen(const unsigned int &radio, const enum espacioColor &espacio)
  {
    this->setRadio(radio);
    this->setEspacioColor(espacio);
    this->setBiecualizacion(false);
  }

  unsigned int EcualizarImagen::getRadio() const
  {
    return _radio;
  }

  void EcualizarImagen::setRadio(const unsigned int &radio)
  {
    _radio = radio;
  }

  bool EcualizarImagen::hayVentanas()
  {
    return this->getRadio() != 0;
  }

  Mat EcualizarImagen::ecualizar(const Mat &imagen, const Mat &mascara)
  {
    assert((mascara.data == NULL) || (mascara.size() == imagen.size()));
    assert((mascara.data == NULL) || (mascara.type() == imagen.type()));

    Mat matrizFinal;

    matrizFinal = imagen.clone();

    if(matrizFinal.channels() == 1) //Imagen monocroma
      {
	if(this->hayVentanas()) //Ecualizacion por ventanas
	  {
	    int radio = static_cast<int>(this->getRadio());
	    for(int i = radio; i < imagen.rows - radio; i++)
	      {
		for(int j = radio; j < imagen.cols - radio; j++)
		  {
		    Mat ventana(imagen, Rect(j - radio, i - radio, radio * 2 + 1, radio * 2 + 1));
		    Mat ventanaSalida(matrizFinal, Rect(j - radio, i - radio, 2 * radio + 1, 2 * radio + 1));
		    Mat ventanaMascara;

		    if(!mascara.empty())
		      {
			ventanaMascara = mascara(Rect(j - radio, i - radio, 2 * radio + 1, 2 * radio + 1));
		      }

		    this->ecualizarVentana(ventana, ventanaSalida, ventanaMascara);
		    
		  }
	      }

	    //Cuando acaban los bucles la imagen no tiene los bordes, debido a que la ventana no
	    //pudo pasar por ellos. Con el rellenado de bordes nos quitamos el problema.
	    //No hago un imagen.clone al principio debido a que, aunque me solucionaria el problema
	    //ya que los bordes estaria copiados, copiaria un monton de datos que voy a cambiar
	    //justo despues.
	    this->rellenarBordes(imagen, matrizFinal);
	  }
	else
	  {
	    this->ecualizarImagen(matrizFinal, mascara);
	  }
      }
    else
      {
	Mat canalIluminacion(imagen.size(), CV_8UC1); //!< Es donde se ira escribiendo la nueva iluminacion
	vector<Mat> canales; //!< Contiene todos los canales de la imagen
	const int ILUMINACION = this->getIluminancia();
	
	this->cambiarEspacioColor(matrizFinal, this->getEspacioColor());
	split(matrizFinal, canales);

	if(this->hayVentanas()) //Ecualizacion cromatica por ventanas
	  {
	    int radio = static_cast<int>(this->getRadio());
	    for(int i = radio; i < canales[ILUMINACION].rows - radio; i++)
	      {
		for(int j = radio; j < canales[ILUMINACION].cols - radio; j++)
		  {
		    int altura = (2 * radio) + 1;
		    Mat ventana(canales[ILUMINACION], Rect(j - radio, i - radio, altura, altura));
		    Mat ventanaSalida(canalIluminacion, Rect(j - radio, i - radio, altura, altura));
		    Mat ventanaMascara;

		    if(!mascara.empty())
		      {
			ventanaMascara = mascara(Rect(j - radio, i - radio, altura, altura));
		      }

		    this->ecualizarVentana(ventana, ventanaSalida, ventanaMascara);
		  }
	      }
	    this->rellenarBordes(canales[ILUMINACION], canalIluminacion);
	    canales[ILUMINACION] = canalIluminacion.clone();
	  }
	else
	  {
	    this->ecualizarImagen(canales[ILUMINACION], mascara);
	  }
	merge(canales, matrizFinal);
	this->cambiarEspacioColor(matrizFinal, ESPACIO_COLOR_RGB);
      }

    return matrizFinal;
  }

  void EcualizarImagen::setEspacioColor(const enum espacioColor &espacio)
  {
    assert(espacio != ESPACIO_COLOR_RGB);
    _espacioColor = espacio;
  }

  enum espacioColor EcualizarImagen::getEspacioColor()
  {
    return _espacioColor;
  }

  void EcualizarImagen::rellenarBordes(const Mat &imagen, Mat &imagenFinal)
  {
    for(unsigned int i = 0; i < this->getRadio(); i++)
      {
	for(unsigned int j = 0; j < static_cast<unsigned int>(imagen.cols); j++)
	  {
	    imagenFinal.at<unsigned char>(i, j) = imagen.at<unsigned char>(i, j);
	  }
      }

    for(unsigned int i = static_cast<unsigned int>(imagen.rows) - this->getRadio(); i < static_cast<unsigned int>(imagen.rows); i++)
      {
	for(unsigned int j = 0; j < static_cast<unsigned int>(imagen.cols); j++)
	  {
	    imagenFinal.at<unsigned char>(i, j) = imagen.at<unsigned char>(i, j);
	  }
      }

    for(unsigned int i = this->getRadio(); i < static_cast<unsigned int>(imagen.rows) - this->getRadio(); i++)
      {
	for(unsigned int j = 0; j < static_cast<unsigned int>(this->getRadio()); j++)
	  {
	    imagenFinal.at<unsigned char>(i, j) = imagen.at<unsigned char>(i, j);
	  }
      }

    for(unsigned int i = this->getRadio(); i < static_cast<unsigned int>(imagen.rows) - this->getRadio(); i++)
      {
	for(unsigned int j = static_cast<unsigned int>(imagen.cols) - this->getRadio(); j < static_cast<unsigned int>(imagen.cols); j++)
	  {
	    imagenFinal.at<unsigned char>(i, j) = imagen.at<unsigned char>(i, j);
	  }
      }
  }

  void EcualizarImagen::ecualizarVentana(const Mat &ventana, Mat &ventanaSalida, const Mat &mascara)
  {
    assert((mascara.empty()) || (ventana.size() == mascara.size()));
    assert((mascara.empty()) || (ventana.type() == mascara.type()));
    assert(ventana.rows > 1);
    assert(ventana.cols > 1);

    HistogramaAcumulado histograma;
    int filaCentral;
    int columnaCentral;

    histograma.procesarDatos(ventana, mascara);
    if(this->getBiecualizacion())
      {
	histograma.binormalizar();
      }
    else
      {
	histograma.normalizar();
      }

    filaCentral = ventana.rows / 2;
    columnaCentral = ventana.cols / 2;

    if((mascara.empty()) || (mascara.at<unsigned char>(filaCentral, columnaCentral) != 0))
      {
	unsigned char valorCentral = ventana.at<unsigned char>(filaCentral, columnaCentral);
       
	ventanaSalida.at<unsigned char>(filaCentral, columnaCentral) = static_cast<unsigned char>(histograma[valorCentral] * 
												  histograma.getMaximo());
      }
  }

  void EcualizarImagen::ecualizarImagen(Mat &imagen, const Mat &mascara)
  {
    HistogramaAcumulado histograma;

    histograma.procesarDatos(imagen, mascara);

    if(this->getBiecualizacion())
      {
	histograma.binormalizar();
      }
    else
      {
	histograma.normalizar();
      }

    for(int i = 0; i < imagen.rows; i++)
      {
	for(int j = 0; j < imagen.cols; j++)
	  {
	    unsigned char valorLeido = imagen.at<unsigned char>(i, j);
	    unsigned char nuevoValor;
	    if((mascara.empty()) || (mascara.at<unsigned char>(i, j) != 0))
	      {
		nuevoValor = static_cast<unsigned char>(histograma[valorLeido] * histograma.getMaximo());

		imagen.at<unsigned char>(i, j) = nuevoValor;
	      }
	  }
      }
  }

  void EcualizarImagen::cambiarEspacioColor(Mat &imagen, const enum espacioColor &espacio)
  {
    if(espacio == ESPACIO_COLOR_HSV)
      {
	cvtColor(imagen, imagen, CV_BGR2HSV);
      }
    else if(espacio == ESPACIO_COLOR_CIE)
      {
	cvtColor(imagen, imagen, CV_BGR2XYZ);
      }
    else if(espacio == ESPACIO_COLOR_YCrCb)
      {
	cvtColor(imagen, imagen, CV_BGR2YCrCb);
      }
    else if(espacio == ESPACIO_COLOR_HSL)
      {
	cvtColor(imagen, imagen, CV_BGR2HLS);
      }
    else //Se trata de restaurar la imagen
      {
	enum espacioColor espacioBase = this->getEspacioColor();
	
	//El espacioBase nunca valdra ESPACIO_COLOR_RGB puesto que
	//el metodo EcualizarImagen::setEspacioColor no lo permite

	if(espacioBase == ESPACIO_COLOR_HSV)
	  {
	    cvtColor(imagen, imagen, CV_HSV2BGR);
	  }
	else if(espacioBase == ESPACIO_COLOR_CIE)
	  {
	    cvtColor(imagen, imagen, CV_XYZ2BGR);
	  }
	else if(espacioBase == ESPACIO_COLOR_YCrCb)
	  {
	    cvtColor(imagen, imagen, CV_YCrCb2BGR);
	  }
	else
	  {
	    cvtColor(imagen, imagen, CV_HLS2BGR);
	  }
      }
  }

  unsigned int EcualizarImagen::getIluminancia()
  {
    enum espacioColor espacioBase = this->getEspacioColor();
    unsigned int canalIluminancia;

    if((espacioBase == ESPACIO_COLOR_HSV) || (espacioBase == ESPACIO_COLOR_HSL))
      {
	canalIluminancia = 2;
      }
    else if(espacioBase == ESPACIO_COLOR_CIE)
      {
	canalIluminancia = 2;
      }
    else
      {
	canalIluminancia = 0;
      }

    return canalIluminancia;
  }

  void EcualizarImagen::setBiecualizacion(const bool &biecualizacion)
  {
    _biecualizacion = biecualizacion;
  }

  bool EcualizarImagen::getBiecualizacion() const
  {
    return _biecualizacion;
  }
}
