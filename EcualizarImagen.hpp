#ifndef __LIB_ECUALIZAR_IMAGEN_HPP
#define __LIB_ECUALIZAR_IMAGEN_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Histograma.hpp"
#include "HistogramaAcumulado.hpp"

using namespace FSIV;
using namespace cv;

namespace FSIV
{
  /** @brief Clase que realiza la ecualización de una imagen. **/
  class EcualizarImagen
  {
  private:
    unsigned int _radio; //!< Radio de la ventana que ecualizara la imagen

  public:
    /** @brief Constructor vacio. Inicializa la imagen con los parametros en blanco.**/
    EcualizarImagen(const unsigned int &radio = 0);

    /** @brief Devuelve el radio asignado
	@return Radio de la mascara. 
    **/
    unsigned int getRadio() const;

    /** @brief Actualiza el radio de la mascara
	@param radio Nuevo radio de la mascara.**/
    void setRadio(const unsigned int &radio);

    /** @brief Indica si es posible la ecualizacion por ventanas
	@return TRUE si es posible ecualizar por ventanas, y FALSE en caso contrario
    **/
    bool hayVentanas();

    /** @brief Realiza la ecualización de la imagen
	@param imagen -->La imagen a ser ecualizada
	@pre Si se inicializo una mascara, deben de coincidir
	@return Una imagen ecualizada
    **/
    Mat ecualizar(const Mat &imagen, const Mat &mascara = cv::Mat());
  };
};

#endif
