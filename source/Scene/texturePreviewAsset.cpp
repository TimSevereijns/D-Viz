#include "texturePreviewAsset.h"

namespace
{
   constexpr auto PROGRAM_VERTEX_ATTRIBUTE{ 0 };
   constexpr auto PROGRAM_TEXCOORD_ATTRIBUTE{ 1 };
}

TexturePreviewAsset::TexturePreviewAsset(GraphicsDevice& graphicsDevice)
   : SceneAsset{ graphicsDevice }
{
}

bool TexturePreviewAsset::Initialize()
{
   static constexpr int coordinates[4][3] =
   {
      { +1, -1, -1 },
      { -1, -1, -1 },
      { -1, +1, -1 },
      { +1, +1, -1 }
   };

   m_shader.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
   m_shader.bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);

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

void TexturePreviewAsset::SetTexture(const QImage& texture)
{
   // Make a copy of the texture and store it.
   m_texture = std::make_unique<QOpenGLTexture>(texture);
}

bool TexturePreviewAsset::LoadShaders()
{
   return SceneAsset::LoadShaders("texturePreview", "texturePreview");
}

bool TexturePreviewAsset::Render(
   const Camera&,
   const std::vector<Light>&,
   const OptionsManager&)
{
   // @note The viewport needs to match the coordinates seen in the `coordinates` array in the
   // Initialize(...) fuction.
   QMatrix4x4 orthoMatrix;
   orthoMatrix.ortho(-1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1000.0f);
   orthoMatrix.translate(0.0f, 0.0f, -1.0f);

   m_VBO.bind();

   m_shader.bind();
   m_shader.setUniformValue("matrix", orthoMatrix);
   m_shader.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
   m_shader.enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);

   m_shader.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT,
      /* offset = */    0,
      /* tupleSize = */ 3,
      /* stride = */    5 * sizeof(GLfloat));

   m_shader.setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT,
      /* offset = */    3 * sizeof(GLfloat),
      /* tupleSize = */ 2,
      /* stride = */    5 * sizeof(GLfloat));

   m_texture->bind();

   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_TRIANGLE_FAN,
      /* first = */ 0,
      /* count = */ 4);

   m_texture->release();
   m_shader.release();

   m_VBO.release();

   return true;
}

bool TexturePreviewAsset::Reload()
{
   return true;
}
