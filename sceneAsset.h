#ifndef SCENEASSET_H
#define SCENEASSET_H

/**
 * @brief The SceneAsset class
 */
class SceneAsset
{
   public:
      explicit SceneAsset();
      ~SceneAsset();

      bool PrepareVertexBuffers();
      bool PrepareColorBuffers();

      bool ClearBuffers();

      bool LoadShaders();
      bool UnloadShaders();

      bool Render();
};

#endif // SCENEASSET_H
