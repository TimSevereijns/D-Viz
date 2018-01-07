#include "texturePreviewAsset.h"

namespace
{
   constexpr auto PROGRAM_VERTEX_ATTRIBUTE{ 0 };
   constexpr auto PROGRAM_TEXCOORD_ATTRIBUTE{ 1 };
}

namespace Asset
{
   TexturePreview::TexturePreview(
      QOpenGLExtraFunctions& openGL,
      bool isInitiallyVisible)
      :
      Base{ openGL, isInitiallyVisible }
   {
   }

   bool TexturePreview::Initialize()
   {
      static constexpr int coordinates[4][3] =
      {
         { +1, -1, -1 },
         { -1, -1, -1 },
         { -1, +1, -1 },
         { +1, +1, -1 }
      };

      m_mainShader.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
      m_mainShader.bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);

      const auto testImageFilePath = QString{ "C:/Users/Tim/Desktop/depth.png" };
      const auto image = QImage{ testImageFilePath };
      m_texture.reset(new QOpenGLTexture(image.mirrored()));

      QVector<GLfloat> vertexData;
      for (int i = 0; i < 4; ++i)
      {
         // Vertex position:
         vertexData.append(coordinates[i][0]);
         vertexData.append(coordinates[i][1]);
         vertexData.append(coordinates[i][2]);

         // Texture coordinate:
         vertexData.append(i == 0 || i == 3);
         vertexData.append(i == 0 || i == 1);
      }

      m_VBO.create();

      m_VBO.bind();
      m_VBO.allocate(vertexData.constData(), vertexData.count() * sizeof(GLfloat));
      m_VBO.release();

      return true;
   }

   void TexturePreview::SetTexture(const QImage& texture)
   {
      // Make a copy of the texture and store it.
      m_texture = std::make_unique<QOpenGLTexture>(texture);
   }

   bool TexturePreview::LoadShaders()
   {
      return Base::LoadShaders("texturePreview", "texturePreview");
   }

   bool TexturePreview::Render(
      const Camera&,
      const std::vector<Light>&,
      const Settings::Manager&)
   {
      // @note The viewport needs to match the coordinates seen in the `coordinates` array in the
      // Initialize(...) fuction.
      QMatrix4x4 orthoMatrix;
      orthoMatrix.ortho(-1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1000.0f);
      orthoMatrix.translate(0.0f, 0.0f, -1.0f);

      m_VBO.bind();

      m_mainShader.bind();
      m_mainShader.setUniformValue("matrix", orthoMatrix);
      m_mainShader.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
      m_mainShader.enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);

      m_mainShader.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT,
         /* offset = */    0,
         /* tupleSize = */ 3,
         /* stride = */    5 * sizeof(GLfloat));

      m_mainShader.setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT,
         /* offset = */    3 * sizeof(GLfloat),
         /* tupleSize = */ 2,
         /* stride = */    5 * sizeof(GLfloat));

      m_texture->bind();

      m_openGL.glDrawArrays(
         /* mode = */ GL_TRIANGLE_FAN,
         /* first = */ 0,
         /* count = */ 4);

      m_texture->release();
      m_mainShader.release();

      m_VBO.release();

      return true;
   }

   bool TexturePreview::Refresh()
   {
      return true;
   }
}
