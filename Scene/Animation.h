#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <map>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

// Forward declarations
struct aiScene;
struct aiNode;

namespace BinRenderer {

using namespace std;
using namespace glm;

/**
 * @brief Animation keyframe template
 * @tparam T Value type (vec3, quat, etc.)
 */
template <typename T>
struct AnimationKey
{
    double time;  // Time in animation (seconds)
    T value;   // Value at this keyframe

AnimationKey() : time(0.0) {}
    AnimationKey(double t, const T& v) : time(t), value(v) {}
};

using PositionKey = AnimationKey<vec3>;
using RotationKey = AnimationKey<quat>;
using ScaleKey = AnimationKey<vec3>;

/**
 * @brief Animation channel for a single node/bone
 */
struct AnimationChannel
{
    string nodeName;  // Target node/bone name

    vector<PositionKey> positionKeys;  // Position keyframes
    vector<RotationKey> rotationKeys;  // Rotation keyframes
    vector<ScaleKey> scaleKeys;        // Scale keyframes

// Interpolation methods
 vec3 interpolatePosition(double time) const;
    quat interpolateRotation(double time) const;
    vec3 interpolateScale(double time) const;

private:
    template <typename T>
    T interpolateKeys(const vector<AnimationKey<T>>& keys, double time) const;
};

/**
 * @brief Bone structure for skeletal animation
 */
struct Bone
{
    string name;              // Bone name
    int id;     // Unique bone ID
    mat4 offsetMatrix;   // Inverse bind pose matrix
    mat4 finalTransformation;   // Final transformation matrix
    int parentIndex;   // Parent bone index (-1 if root)

    struct VertexWeight
    {
        uint32_t vertexId;
        float weight;
    };
    vector<VertexWeight> weights;

    Bone() : id(-1), offsetMatrix(1.0f), finalTransformation(1.0f), parentIndex(-1) {}
};

/**
 * @brief Platform-independent Animation system
 * 
 * NO Vulkan dependencies - pure logic class
 */
class Animation
{
public:
  Animation();
    ~Animation();

    // Animation loading (platform-independent)
    void loadFromScene(const aiScene* scene);
    void processAnimations(const aiScene* scene);
  void processBones(const aiScene* scene);
    void buildSceneGraph(const aiScene* scene);
    void buildBoneHierarchy(const aiScene* scene);
    void assignGlobalBoneIds();

    // Animation playback
    void updateAnimation(float timeInSeconds);
    void setAnimationIndex(uint32_t index);
    void setPlaybackSpeed(float speed);
    void setLooping(bool loop);

    // Bone transformation calculation
    void calculateBoneTransforms(vector<mat4>& transforms, 
           const string& nodeName = "",
        const mat4& parentTransform = mat4(1.0f));
    mat4 getNodeTransformation(const string& nodeName, double time) const;
    int getGlobalBoneIndex(const string& boneName) const;

    // State queries
    bool hasAnimations() const { return !animations_.empty(); }
    bool hasBones() const { return !bones_.empty(); }
    uint32_t getAnimationCount() const { return static_cast<uint32_t>(animations_.size()); }
    uint32_t getBoneCount() const { return static_cast<uint32_t>(bones_.size()); }
    float getDuration() const;
    float getCurrentTime() const { return currentTime_; }
    const string& getCurrentAnimationName() const;

 // Bone matrices for GPU upload
  const vector<mat4>& getBoneMatrices() const { return boneMatrices_; }
    const mat4& getGlobalInverseTransform() const { return globalInverseTransform_; }
    void setGlobalInverseTransform(const mat4& transform) { globalInverseTransform_ = transform; }

    // Playback control
    bool isPlaying() const { return isPlaying_; }
    void play() { isPlaying_ = true; }
    void pause() { isPlaying_ = false; }
    void stop() { currentTime_ = 0.0f; isPlaying_ = false; }

private:
    struct AnimationData
  {
     string name;
        double duration;         // Animation duration in seconds
        double ticksPerSecond;   // Ticks per second
      vector<AnimationChannel> channels;
    };

    struct SceneNode
    {
     string name;
        mat4 transformation;
   int parentIndex;
        vector<int> childIndices;

SceneNode() : transformation(1.0f), parentIndex(-1) {}
    };

    // Animation data
    vector<AnimationData> animations_;
    uint32_t currentAnimationIndex_ = 0;
    float currentTime_ = 0.0f;
    float playbackSpeed_ = 1.0f;
    bool isPlaying_ = false;
    bool isLooping_ = true;

    // Bone data
    vector<Bone> bones_;
    unordered_map<string, int> boneMapping_;  // name -> bone index
 vector<mat4> boneMatrices_;
    mat4 globalInverseTransform_;

    // Scene graph
    vector<SceneNode> sceneNodes_;
    unordered_map<string, int> nodeMapping_;  // name -> node index

    // Helper methods
    void updateBoneMatrices();
    const AnimationChannel* findChannel(const string& nodeName) const;
};

} // namespace BinRenderer
