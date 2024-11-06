#include "Block.hpp"

Block::Block(glm::vec3 init_position, glm::vec3 block_scale, std::shared_ptr<Mona::DiffuseFlatMaterial> block_material,
        Mona::TransformHandle ball_transform, float ball_radius, Mona::GameObjectHandle<Ball> ball) :
    ball_transform(ball_transform), ball_radius(ball_radius), position(init_position), scale(block_scale), material(block_material), ball(ball) {}

Block::~Block() = default;

void Block::UserStartUp(Mona::World& world) noexcept {
    auto& audioClipManager = Mona::AudioClipManager::GetInstance();
    Mona::Config& config = Mona::Config::GetInstance();
    m_blockBreakingSound = audioClipManager.LoadAudioClip(config.getPathOfApplicationAsset("audio/boxBreaking.wav"));
    auto& meshManager = Mona::MeshManager::GetInstance();
    auto transform = world.AddComponent<Mona::TransformComponent>(*this, position);
    transform->Scale(scale);
    world.AddComponent<Mona::StaticMeshComponent>(*this, meshManager.LoadMesh(Mona::Mesh::PrimitiveType::Cube), material);
}

// Reaccionar a la colisión con la bola
void Block::OnCollisionWithBall(Mona::World& world) {
    world.PlayAudioClip2D(m_blockBreakingSound, 1.0f, 1.0f);
    world.DestroyGameObject(*this);
}

// Revisa la colisión con el paddle
void Block::checkBallCollision(Mona::World& world) {
    auto ballPos = ball_transform->GetLocalTranslation();
    // Primero revisamos con un bounding box, si no colisiona con el bounding box no colisiona con el paddle
    if (ballPos.x + ball_radius < position.x - scale.x || ballPos.x - ball_radius > position.x + scale.x) {
        return;
    }
    if (ballPos.y + ball_radius < position.y - scale.y || ballPos.y - ball_radius > position.y + scale.y) {
        return;
    }
    if (ballPos.z + ball_radius < position.z - scale.z || ballPos.z - ball_radius > position.z + scale.z) { // no debería pasar
        return;
    }
    // Si colisiona con el bounding box, revisamos si el punto más cercano del paddle a la bola está dentro de la bola
    float closestPoint_x = glm::clamp(ballPos.x, position.x - scale.x, position.x + scale.x);
    float closestPoint_y = glm::clamp(ballPos.y, position.y - scale.y, position.y + scale.y);
    float closestPoint_z = glm::clamp(ballPos.z, position.z - scale.z, position.z + scale.z);

    float distance = glm::distance(glm::vec3(closestPoint_x, closestPoint_y, closestPoint_z), glm::vec3(ballPos.x, ballPos.y, ballPos.z));

    if (distance > ball_radius) {
        return;
    }

    // Si el punto más cercano está dentro de la bola, entonces la bola colisionó con el paddle
    // Rebotar con dirección según la posición de la colisión
    glm::vec3 normal;
    if (closestPoint_x == position.x - scale.x) {
        normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    else if (closestPoint_x == position.x + scale.x) {
        normal = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    else if (closestPoint_y == position.y - scale.y) {
        normal = glm::vec3(0.0f, -1.0f, 0.0f);
    }
    else if (closestPoint_y == position.y + scale.y) {
        normal = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    ball->OnCollisionBall(world, glm::vec3(closestPoint_x, closestPoint_y, closestPoint_z), normal);
    OnCollisionWithBall(world);
}

void Block::UserUpdate(Mona::World& world, float timeStep) noexcept {
    // Colisiones con el paddle
    checkBallCollision(world);
}