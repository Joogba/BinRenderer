#include "Animation.h"
#include "../Core/Logger.h"

#include <algorithm>
#include <functional>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

namespace BinRenderer {

Animation::Animation()
    : currentAnimationIndex_(0), currentTime_(0.0f), playbackSpeed_(1.0f), 
  isPlaying_(false), isLooping_(true), globalInverseTransform_(1.0f)
{
}

Animation::~Animation() = default;

void Animation::loadFromScene(const aiScene* scene)
{
    if (!scene) {
    printLog("Animation::loadFromScene - Invalid scene");
     return;
    }

    printLog("Loading animation data from scene...");
    printLog("  Animations found: {}", scene->mNumAnimations);

    // Store global inverse transform
    if (scene->mRootNode) {
 globalInverseTransform_ = 
            glm::inverse(glm::transpose(glm::make_mat4(&scene->mRootNode->mTransformation.a1)));
    }

    buildSceneGraph(scene);
    processBones(scene);
    buildBoneHierarchy(scene);
    assignGlobalBoneIds();

    if (scene->mNumAnimations > 0) {
        processAnimations(scene);
    }

    // Initialize bone matrices
    boneMatrices_.resize(bones_.size(), mat4(1.0f));

    printLog("Animation loading complete:");
 printLog("  Animation clips: {}", animations_.size());
 printLog("  Bones: {}", bones_.size());
    printLog("  Scene nodes: {}", nodeMapping_.size());
}

void Animation::processBones(const aiScene* scene)
{
    if (!scene)
        return;

    printLog("Processing bones...");

    // Collect all unique bones from all meshes
    unordered_map<string, mat4> boneOffsetMatrices;
    unordered_map<string, vector<Bone::VertexWeight>> boneWeights;

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];

        if (!mesh->HasBones())
            continue;

   for (uint32_t boneIdx = 0; boneIdx < mesh->mNumBones; ++boneIdx) {
     const aiBone* aiBone = mesh->mBones[boneIdx];
  string boneName = aiBone->mName.C_Str();

            // Store offset matrix
            if (boneOffsetMatrices.find(boneName) == boneOffsetMatrices.end()) {
        boneOffsetMatrices[boneName] = 
         glm::transpose(glm::make_mat4(&aiBone->mOffsetMatrix.a1));
            }

            // Collect vertex weights
       for (uint32_t weightIdx = 0; weightIdx < aiBone->mNumWeights; ++weightIdx) {
     const aiVertexWeight& weight = aiBone->mWeights[weightIdx];
        boneWeights[boneName].push_back({weight.mVertexId, weight.mWeight});
}
        }
    }

    // Create global bone list
    bones_.clear();
    boneMapping_.clear();

    uint32_t globalBoneIndex = 0;
for (const auto& [boneName, offsetMatrix] : boneOffsetMatrices) {
Bone bone;
        bone.name = boneName;
        bone.id = globalBoneIndex;
        bone.offsetMatrix = offsetMatrix;
        bone.weights = boneWeights[boneName];

 bones_.push_back(bone);
        boneMapping_[boneName] = globalBoneIndex;

   globalBoneIndex++;
    }

    printLog("Created {} global bones", bones_.size());
}

void Animation::buildBoneHierarchy(const aiScene* scene)
{
    if (!scene || !scene->mRootNode)
        return;

    // Reset parent indices
    for (auto& bone : bones_) {
        bone.parentIndex = -1;
    }

    // Function to find parent bone recursively
    std::function<const aiNode*(const aiNode*)> findBoneParent = 
    [&](const aiNode* node) -> const aiNode* {
        if (!node || !node->mParent)
  return nullptr;

        if (boneMapping_.find(node->mParent->mName.C_Str()) != boneMapping_.end()) {
            return node->mParent;
        }

  return findBoneParent(node->mParent);
    };

    // Traverse scene nodes and establish hierarchy
    std::function<void(const aiNode*)> traverseNodes = [&](const aiNode* node) {
        if (!node)
return;

      string nodeName = node->mName.C_Str();

        // If this node represents a bone
  if (boneMapping_.find(nodeName) != boneMapping_.end()) {
   int boneIndex = boneMapping_[nodeName];

            // Find parent bone
    const aiNode* parentBone = findBoneParent(node);
      if (parentBone) {
     string parentName = parentBone->mName.C_Str();
 if (boneMapping_.find(parentName) != boneMapping_.end()) {
     int parentIndex = boneMapping_[parentName];
           bones_[boneIndex].parentIndex = parentIndex;
         }
   }
        }

        // Process children
 for (uint32_t i = 0; i < node->mNumChildren; ++i) {
 traverseNodes(node->mChildren[i]);
      }
    };

    traverseNodes(scene->mRootNode);
}

void Animation::assignGlobalBoneIds()
{
  printLog("Global bone ID assignment complete: {} bones", bones_.size());
}

int Animation::getGlobalBoneIndex(const string& boneName) const
{
    auto it = boneMapping_.find(boneName);
    return (it != boneMapping_.end()) ? it->second : -1;
}

void Animation::processAnimations(const aiScene* scene)
{
    animations_.resize(scene->mNumAnimations);

  for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
        const aiAnimation* aiAnim = scene->mAnimations[i];

        AnimationData& anim = animations_[i];
        anim.name = aiAnim->mName.C_Str();
        anim.duration = aiAnim->mDuration;
        anim.ticksPerSecond = aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 25.0;

        // Process animation channels
        anim.channels.reserve(aiAnim->mNumChannels);
        for (uint32_t j = 0; j < aiAnim->mNumChannels; ++j) {
     const aiNodeAnim* nodeAnim = aiAnim->mChannels[j];

   AnimationChannel channel;
     channel.nodeName = nodeAnim->mNodeName.C_Str();

            // Extract position keys
            channel.positionKeys.reserve(nodeAnim->mNumPositionKeys);
            for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; ++k) {
       const aiVectorKey& key = nodeAnim->mPositionKeys[k];
      channel.positionKeys.emplace_back(
             key.mTime, vec3(key.mValue.x, key.mValue.y, key.mValue.z));
       }

         // Extract rotation keys
    channel.rotationKeys.reserve(nodeAnim->mNumRotationKeys);
            for (uint32_t k = 0; k < nodeAnim->mNumRotationKeys; ++k) {
    const aiQuatKey& key = nodeAnim->mRotationKeys[k];
         channel.rotationKeys.emplace_back(
            key.mTime, quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
            }

    // Extract scale keys
            channel.scaleKeys.reserve(nodeAnim->mNumScalingKeys);
   for (uint32_t k = 0; k < nodeAnim->mNumScalingKeys; ++k) {
       const aiVectorKey& key = nodeAnim->mScalingKeys[k];
           channel.scaleKeys.emplace_back(
              key.mTime, vec3(key.mValue.x, key.mValue.y, key.mValue.z));
   }

  anim.channels.push_back(std::move(channel));
        }
    }
}

void Animation::buildSceneGraph(const aiScene* scene)
{
    if (!scene || !scene->mRootNode)
        return;

    printLog("Building scene graph...");
    
  sceneNodes_.clear();
    nodeMapping_.clear();

    // Build flat list of nodes
    std::function<void(const aiNode*, int)> traverseNode = 
      [&](const aiNode* aiNode, int parentIdx) {
        int currentIdx = static_cast<int>(sceneNodes_.size());
        
    SceneNode node;
        node.name = aiNode->mName.C_Str();
      node.transformation = glm::transpose(glm::make_mat4(&aiNode->mTransformation.a1));
        node.parentIndex = parentIdx;

        sceneNodes_.push_back(node);
        nodeMapping_[node.name] = currentIdx;

   // Update parent's child list
        if (parentIdx >= 0) {
 sceneNodes_[parentIdx].childIndices.push_back(currentIdx);
        }

        // Process children
        for (uint32_t i = 0; i < aiNode->mNumChildren; ++i) {
            traverseNode(aiNode->mChildren[i], currentIdx);
        }
    };

    traverseNode(scene->mRootNode, -1);
    printLog("Scene graph built with {} nodes", sceneNodes_.size());
}

void Animation::updateAnimation(float deltaTime)
{
    if (!isPlaying_ || animations_.empty())
        return;

    currentTime_ += deltaTime * playbackSpeed_;

    const AnimationData& currentAnim = animations_[currentAnimationIndex_];
    double animationTime = currentTime_ * currentAnim.ticksPerSecond;

    // Handle looping
    if (isLooping_ && animationTime > currentAnim.duration) {
     currentTime_ = 0.0f;
        animationTime = 0.0;
  } else if (!isLooping_ && animationTime > currentAnim.duration) {
        currentTime_ = static_cast<float>(currentAnim.duration / currentAnim.ticksPerSecond);
        animationTime = currentAnim.duration;
      isPlaying_ = false;
 }

    // Update bone transformations
    updateBoneMatrices();
}

void Animation::updateBoneMatrices()
{
    if (animations_.empty() || sceneNodes_.empty())
        return;

    const AnimationData& currentAnim = animations_[currentAnimationIndex_];
    double animationTime = currentTime_ * currentAnim.ticksPerSecond;

    // Traverse scene graph starting from root
    calculateBoneTransforms(boneMatrices_, "", mat4(1.0f));
}

void Animation::calculateBoneTransforms(vector<mat4>& transforms, 
   const string& nodeName,
 const mat4& parentTransform)
{
    // Start from root if nodeName is empty
    int nodeIdx = nodeName.empty() ? 0 : (nodeMapping_.count(nodeName) ? nodeMapping_[nodeName] : -1);
    
    if (nodeIdx < 0)
   return;

    const SceneNode& node = sceneNodes_[nodeIdx];

    // Get animated transformation
    mat4 nodeTransform = getNodeTransformation(node.name, 
        currentTime_ * animations_[currentAnimationIndex_].ticksPerSecond);

    // If no animation channel, use original transform
    if (nodeTransform == mat4(1.0f)) {
  nodeTransform = node.transformation;
    }

    mat4 globalTransform = parentTransform * nodeTransform;

    // If this is a bone, compute final transformation
    if (boneMapping_.count(node.name)) {
        int boneIdx = boneMapping_[node.name];
        if (boneIdx >= 0 && boneIdx < static_cast<int>(bones_.size())) {
   transforms[boneIdx] = 
             globalInverseTransform_ * globalTransform * bones_[boneIdx].offsetMatrix;
     }
    }

    // Process children
    for (int childIdx : node.childIndices) {
        if (childIdx >= 0 && childIdx < static_cast<int>(sceneNodes_.size())) {
    calculateBoneTransforms(transforms, sceneNodes_[childIdx].name, globalTransform);
     }
    }
}

mat4 Animation::getNodeTransformation(const string& nodeName, double time) const
{
    if (animations_.empty())
        return mat4(1.0f);

    const AnimationData& currentAnim = animations_[currentAnimationIndex_];
    const AnimationChannel* channel = findChannel(nodeName);

    if (!channel)
     return mat4(1.0f);

    vec3 position = channel->interpolatePosition(time);
    quat rotation = channel->interpolateRotation(time);
    vec3 scale = channel->interpolateScale(time);

    mat4 translation = glm::translate(mat4(1.0f), position);
    mat4 rotationMat = glm::mat4_cast(rotation);
  mat4 scaleMat = glm::scale(mat4(1.0f), scale);

    return translation * rotationMat * scaleMat;
}

const AnimationChannel* Animation::findChannel(const string& nodeName) const
{
    if (currentAnimationIndex_ >= animations_.size())
        return nullptr;

    const AnimationData& currentAnim = animations_[currentAnimationIndex_];
    for (const auto& channel : currentAnim.channels) {
      if (channel.nodeName == nodeName) {
 return &channel;
        }
    }
    return nullptr;
}

float Animation::getDuration() const
{
    if (animations_.empty())
   return 0.0f;
  const auto& currentAnim = animations_[currentAnimationIndex_];
    return static_cast<float>(currentAnim.duration / currentAnim.ticksPerSecond);
}

const string& Animation::getCurrentAnimationName() const
{
    static const string empty = "";
    if (animations_.empty())
        return empty;
    return animations_[currentAnimationIndex_].name;
}

void Animation::setAnimationIndex(uint32_t index)
{
    if (index < animations_.size()) {
        currentAnimationIndex_ = index;
        currentTime_ = 0.0f;
    }
}

void Animation::setPlaybackSpeed(float speed)
{
    playbackSpeed_ = speed;
}

void Animation::setLooping(bool loop)
{
    isLooping_ = loop;
}

// AnimationChannel interpolation methods
vec3 AnimationChannel::interpolatePosition(double time) const
{
    return interpolateKeys(positionKeys, time);
}

quat AnimationChannel::interpolateRotation(double time) const
{
    if (rotationKeys.empty())
        return quat(1.0f, 0.0f, 0.0f, 0.0f);
    if (rotationKeys.size() == 1)
        return rotationKeys[0].value;

    // Find surrounding keyframes
    uint32_t index = 0;
    for (uint32_t i = 0; i < rotationKeys.size() - 1; ++i) {
    if (time < rotationKeys[i + 1].time) {
   index = i;
            break;
        }
    }

    if (index >= rotationKeys.size() - 1) {
        return rotationKeys.back().value;
    }

    const auto& key1 = rotationKeys[index];
    const auto& key2 = rotationKeys[index + 1];

    double deltaTime = key2.time - key1.time;
    float factor = static_cast<float>((time - key1.time) / deltaTime);

    return glm::slerp(key1.value, key2.value, factor);
}

vec3 AnimationChannel::interpolateScale(double time) const
{
    return interpolateKeys(scaleKeys, time);
}

template <typename T>
T AnimationChannel::interpolateKeys(const vector<AnimationKey<T>>& keys, double time) const
{
    if (keys.empty())
        return T{};
    if (keys.size() == 1)
        return keys[0].value;

    // Find surrounding keyframes
    uint32_t index = 0;
    for (uint32_t i = 0; i < keys.size() - 1; ++i) {
        if (time < keys[i + 1].time) {
        index = i;
        break;
        }
    }

  if (index >= keys.size() - 1) {
  return keys.back().value;
    }

    const auto& key1 = keys[index];
    const auto& key2 = keys[index + 1];

    double deltaTime = key2.time - key1.time;
    float factor = static_cast<float>((time - key1.time) / deltaTime);

    return glm::mix(key1.value, key2.value, factor);
}

} // namespace BinRenderer
