#include <cv.h>
#include <mlhelper/idl/Matlab.hh>
#include <mclmcr.h>
#include <mclcppclass.h>

class CMatlabHelper
{
 public:
   // Create a Matrix from a mwArray
   static void array2idl(mwArray &array, Matlab::Matrix& _matrix);

   // Create a mwArray from a Matrix
   static mwArray idl2array(const Matlab::Matrix& _matrix);
 
   // Create a mwArray from an IplImage
   static mwArray iplImage2array(IplImage *image);
   static mwArray iplImage2array(unsigned char *pImageData, unsigned width, unsigned height, 
      unsigned nChannels, unsigned align = 4);
 
   /// @param pImageData Pointer to source data, the IplImage raw image bytes.
   /// @param pArrayData Pointer to destination data, raw array data. If the value is NULL, it will be allocated.
   ///
   /// Assumption: 
   ///    @p pImageData is an interleaved 8-bit image with @p align -byte alignment of rows.
   ///    The order of channels in a color image is BGR.
   /// Assumption: 
   ///    @pArrayData array stores data column by column, channels are separated.
   ///    The order of channels in a color image is RGB.
   ///
   /// @return The number of elements in @p pArrayData.
  template<typename TMatElem>
  static unsigned iplImage2rawArray(unsigned char *pImageData, TMatElem *&pArrayData,
      unsigned width, unsigned height, unsigned nChannels, unsigned align = 4);
 
  static void iplImage2matrix(unsigned char *pImageData, Matlab::Matrix& _matrix,
      unsigned width, unsigned height, unsigned nChannels, unsigned align = 4);

   // Prepare 1d vector matrix;
   template<typename T>
   static void sequence2matrix(const T* _data, unsigned _length, 
		       Matlab::Matrix& _matrix);
 
   // Prepare 1d vector matrix;
   template<typename T>
   static void sequence2matrix(const T* _data, Matlab::Matrix& _matrix, 
		       unsigned _width, unsigned _height = 1, unsigned _depth = 1);

  //		template<typename T>
  //			void matrix2sequence(const Matlab::Matrix& _matrix, T* _data);

};

template<typename TMatElem>
unsigned CMatlabHelper::iplImage2rawArray(unsigned char *pImageData, TMatElem *&pArrayData,
   unsigned width, unsigned height, unsigned nChannels, unsigned align)
{
   if (align < 1) align = 1;
   unsigned stride = ((width * nChannels * sizeof(unsigned char) + align - 1) / align) * align;
   unsigned nPixelsInPlane = width * height;
   unsigned nArrayElements = nPixelsInPlane * nChannels;
   if (pArrayData == NULL) pArrayData = new TMatElem[nArrayElements];
   for (unsigned ch = 0; ch < nChannels; ch++) {
      for (unsigned y = 0; y < height; y++) {
         // channel ch of first pixel in row y
         unsigned char *pIplData = pImageData + stride * y + ch ;
         // row-col swapping, channel separation, channel mixing (BGR->RGB)
         TMatElem *pDst = pArrayData + y + (nChannels - ch - 1) * nPixelsInPlane;
         for (unsigned x = 0; x < width; x++) {
            *pDst = *pIplData;
            pIplData += nChannels;
            pDst += height; // next column, same row
         }
      }
   }
   return nArrayElements;
}

template<typename T>
void CMatlabHelper::sequence2matrix(const T* _data, unsigned _length,
				   Matlab::Matrix& _matrix)
{
   // Set the dimensions.
   _matrix.dimensions_.length(2);
   _matrix.dimensions_[0] = 1;
   _matrix.dimensions_[1] = _length;

   // Set the data.
   _matrix.data_.length(_length);
   for (unsigned i = 0; i < _length; i++) {
      _matrix.data_[i] = _data[i];
   }
}

template<typename T>
void CMatlabHelper::sequence2matrix(const T* _data, Matlab::Matrix& _matrix, 
				   unsigned _width, unsigned _height /*= 1*/, unsigned _depth /*= 1*/)
{
   unsigned dataLength = _width * _height * _depth;

   // Set the dimensions.
   _matrix.dimensions_.length(3);
   _matrix.dimensions_[0] = _height;
   _matrix.dimensions_[1] = _width;
   _matrix.dimensions_[2] = _depth;
   _matrix.data_.length(dataLength);

   unsigned planeSize = _width * _height;

   const unsigned char* pData = _data;
   double* pResult = &(_matrix.data_[0]);

   for (unsigned c = 0; c < _depth; c++, 
         pData += planeSize, pResult += planeSize)
   {
      for (unsigned y = 0; y < _height; y++) {
         for (unsigned x = 0; x < _width; x++) {
            *(pResult + x * _height + y) = *(pData + y * _width + x);
         }
      }
   }
}
