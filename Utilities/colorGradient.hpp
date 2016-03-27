#ifndef COLORGRADIENT
#define COLORGRADIENT

#include <algorithm>
#include <assert.h>
#include <vector>

#include <QVector3D>

//
// Inspired by: http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
//

namespace
{
   /**
    * @brief The ColorPoint struct is an internal class used to store colors at different points
    * in the gradient
    */
   struct ColorPoint
   {
      ColorPoint(float _red, float _green, float _blue, float value) :
         red(_red),
         green(_green),
         blue(_blue),
         normalizedValue(value)
      {
      }

      float red;
      float green;
      float blue;

      float normalizedValue;
   };
}

/**
 * @brief The ColorGradient class
 */
class ColorGradient
{
   public:
      ColorGradient() :
         m_colorPoints
         ({
            ColorPoint( 0, 0, 1, 0.0000f ), // Blue
            ColorPoint( 0, 1, 1, 0.0005f ), // Cyan
            ColorPoint( 0, 1, 0, 0.0010f ), // Green
            ColorPoint( 1, 1, 0, 0.0020f ), // Yellow
            ColorPoint( 1, 0, 0, 1.0000f )  // Red
         })
      {
      }

      /**
       * @brief Inserts a new ColorPoint at the correct poisition into the gradient.
       *
       * @param red
       * @param green
       * @param blue
       * @param value
       */
      void AddColorPoint(float red, float green, float blue, float value)
      {
         for(int i = 0; i < m_colorPoints.size(); i++)
         {
            if (value < m_colorPoints[i].normalizedValue)
            {
               m_colorPoints.insert(std::begin(m_colorPoints) + i, ColorPoint{ red, green, blue, value });
               return;
            }
         }

         m_colorPoints.emplace_back(ColorPoint{ red, green, blue, value });
      }

      /**
       * @brief Clears the current gradient.
       */
      void ClearGradient()
      {
         m_colorPoints.clear();
      }

      /**
       * @brief GetColorAtValue
       *
       * @param[in] value           An input value between 0 and 1.
       *
       * @returns The color corresponding to the input value on the gradient.
       */
      QVector3D GetColorAtValue(const float value)
      {
         assert(m_colorPoints.size());
         if (m_colorPoints.size() == 0)
         {
            return { 1.0f, 1.0f, 1.0f };
         }

         for (int i = 0; i < m_colorPoints.size(); i++)
         {
            const ColorPoint& currentColor = m_colorPoints[i];

            if (value < currentColor.normalizedValue)
            {
               const ColorPoint& previousColor  = m_colorPoints[std::max(0, i - 1)];
               const float delta    = previousColor.normalizedValue - currentColor.normalizedValue;
               const float fractBetween = (delta == 0) ? 0 : (value - currentColor.normalizedValue) / delta;

               const QVector3D finalColor
               {
                  (previousColor.red - currentColor.red) * fractBetween + currentColor.red,
                  (previousColor.green - currentColor.green) * fractBetween + currentColor.green,
                  (previousColor.blue - currentColor.blue) * fractBetween + currentColor.blue

               };

               return finalColor;
             }
         }

         const QVector3D finalColor
         {
            m_colorPoints.back().red,
            m_colorPoints.back().green,
            m_colorPoints.back().blue
         };

         return finalColor;
      }

      std::vector<ColorPoint> m_colorPoints;      ///< Contains the points in ascending order.
};

#endif // COLORGRADIENT
