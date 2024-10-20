#include "Ball.hpp"

Ball::Ball(Mona::TransformHandle transform) : m_transform(transform) {}

Ball::~Ball() = default;

void Ball::UserStartUp(Mona::World& world) noexcept {
    auto& audioClipManager = Mona::AudioClipManager::GetInstance();
    Mona::Config& config = Mona::Config::GetInstance();
    m_ballBounceSound = audioClipManager.LoadAudioClip(config.getPathOfApplicationAsset("audio/ballBounce.wav"));
    auto& meshManager = Mona::MeshManager::GetInstance();
    // Crear la bola, su material y su forma.
    auto ball = world.CreateGameObject<Mona::GameObject>();
    m_ballRadius = 0.5f;
    m_ballTransform = world.AddComponent<Mona::TransformComponent>(ball);
    m_ballTransform->SetRotation(m_transform->GetLocalRotation());
    m_ballTransform->SetTranslation(m_transform->GetLocalTranslation() + glm::vec3(0.0f, 2.0f, 0.0f));
    m_ballTransform->SetScale(glm::vec3(m_ballRadius));
    auto ballMaterial = std::static_pointer_cast<Mona::DiffuseFlatMaterial>(world.CreateMaterial(Mona::MaterialType::DiffuseFlat));
    ballMaterial->SetDiffuseColor(glm::vec3(0.75f, 0.3f, 0.3f));
    world.AddComponent<Mona::StaticMeshComponent>(ball, meshManager.LoadMesh(Mona::Mesh::PrimitiveType::Sphere), ballMaterial);
    // Vector velocidad de la bola
    m_ballVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
}
// Reaccionar a la colisión de la bola
void Ball::OnCollisionBall(Mona::World& world, glm::vec3 collision_pos, glm::vec3 other_normal) {
    world.PlayAudioClip3D(m_ballBounceSound, m_ballTransform->GetLocalTranslation(), 0.3f);
    // m_ballTransform->SetTranslation(collision_pos + other_normal * m_ballRadius);
    m_ballVelocity = glm::reflect(m_ballVelocity, other_normal);
}
// Revisa la colisión con el paddle
void Ball::checkPaddleCollision(Mona::World& world, Mona::TransformHandle paddleTransform) {
    if (time_since_last_collision < 1.0f) { // Evitar interacciones múltiples en la misma colisión
        return;
    }
    auto paddlePos = paddleTransform->GetLocalTranslation();
    auto paddleScale = paddleTransform->GetLocalScale();
    auto ballPos = m_ballTransform->GetLocalTranslation();
    // Primero revisamos con un bounding box, si no colisiona con el bounding box no colisiona con el paddle
    if (ballPos.x + m_ballRadius < paddlePos.x - paddleScale.x || ballPos.x - m_ballRadius > paddlePos.x + paddleScale.x) {
        return;
    }
    if (ballPos.y + m_ballRadius < paddlePos.y - paddleScale.y || ballPos.y - m_ballRadius > paddlePos.y + paddleScale.y) {
        return;
    }
    if (ballPos.z + m_ballRadius < paddlePos.z - paddleScale.z || ballPos.z - m_ballRadius > paddlePos.z + paddleScale.z) { // no debería pasar
        return;
    }
    // Si colisiona con el bounding box, revisamos si el punto más cercano del paddle a la bola está dentro de la bola
    float closestPoint_x = glm::clamp(ballPos.x, paddlePos.x - paddleScale.x, paddlePos.x + paddleScale.x);
    float closestPoint_y = glm::clamp(ballPos.y, paddlePos.y - paddleScale.y, paddlePos.y + paddleScale.y);
    float closestPoint_z = glm::clamp(ballPos.z, paddlePos.z - paddleScale.z, paddlePos.z + paddleScale.z);
    float distance = glm::distance(glm::vec3(closestPoint_x, closestPoint_y, closestPoint_z), glm::vec3(ballPos.x, ballPos.y, ballPos.z));
    if (distance > m_ballRadius) {
        return;
    }
    // Si el punto más cercano está dentro de la bola, entonces la bola colisionó con el paddle, rebotar con dirección según la posición de la colisión
    time_since_last_collision = 0.0f; // Reiniciar el tiempo desde la última colisión
    glm::vec3 new_direction = glm::normalize(glm::vec3((closestPoint_x - paddlePos.x)*5, -m_ballVelocity.y, 0.0f));
    m_ballVelocity = new_direction * 15.0f;
}
void Ball::UserUpdate(Mona::World& world, float timeStep) noexcept {
    auto& input = world.GetInput();
    // Lanzar la bola para empezar el juego
    if (input.IsMouseButtonPressed(MONA_MOUSE_BUTTON_1) || input.IsGamepadButtonPressed(MONA_JOYSTICK_1, MONA_GAMEPAD_BUTTON_A)) {
        if (m_ballVelocity == glm::vec3(0.0f, 0.0f, 0.0f))
            m_ballVelocity = glm::vec3(0.0f, 15.0f, 0.0f);
    }
    // Actualizar la posición de la bola
    m_ballTransform->SetTranslation(m_ballTransform->GetLocalTranslation() + m_ballVelocity * timeStep);
    // Colisiones con el paddle
    checkPaddleCollision(world, m_transform);
    if (time_since_last_collision < 2.0f) {
        time_since_last_collision += timeStep;
    }
    // Colisiones con las paredes
    if (m_ballTransform->GetLocalTranslation().x < -18.0f) {
        OnCollisionBall(world, glm::vec3(-18.0f, m_ballTransform->GetLocalTranslation().y, m_ballTransform->GetLocalTranslation().z), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else if (m_ballTransform->GetLocalTranslation().x > 18.0f) {
        OnCollisionBall(world, glm::vec3(18.0f, m_ballTransform->GetLocalTranslation().y, m_ballTransform->GetLocalTranslation().z), glm::vec3(-1.0f, 0.0f, 0.0f));
    }
    else if (m_ballTransform->GetLocalTranslation().y < -4.0f) {
        OnCollisionBall(world, glm::vec3(m_ballTransform->GetLocalTranslation().x, -4.0f, m_ballTransform->GetLocalTranslation().z), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (m_ballTransform->GetLocalTranslation().y > 25.0f) {
        OnCollisionBall(world, glm::vec3(m_ballTransform->GetLocalTranslation().x, 25.0f, m_ballTransform->GetLocalTranslation().z), glm::vec3(0.0f, -1.0f, 0.0f));
    }
}
float Ball::getBallRadius() const noexcept {
    return m_ballRadius;
}
Mona::TransformHandle Ball::GetTransform() const noexcept {
    return m_ballTransform;
}

