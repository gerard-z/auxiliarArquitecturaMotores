#include "Paddle.hpp"

Paddle::Paddle(float velocity) : m_paddleVelocity(velocity) {}

Paddle::~Paddle() = default;
void Paddle::UserStartUp(Mona::World& world) noexcept {
    m_transform = world.AddComponent<Mona::TransformComponent>(*this);
    glm::vec3 paddleScale(2.0f, 0.5f, 0.5f);
    m_transform->Scale(paddleScale);
    auto paddleMaterial = std::static_pointer_cast<Mona::DiffuseFlatMaterial>(world.CreateMaterial(Mona::MaterialType::DiffuseFlat));
    paddleMaterial->SetDiffuseColor(glm::vec3(0.3f, 0.3f, 0.75f));
    auto& meshManager = Mona::MeshManager::GetInstance();
    world.AddComponent<Mona::StaticMeshComponent>(*this, meshManager.LoadMesh(Mona::Mesh::PrimitiveType::Cube), paddleMaterial);
}

void Paddle::UserUpdate(Mona::World& world, float timeStep) noexcept {
    auto& input = world.GetInput();
    if (input.IsKeyPressed(MONA_KEY_A) || input.IsKeyPressed(MONA_KEY_LEFT) 
        || input.IsGamepadButtonPressed(MONA_JOYSTICK_1, MONA_GAMEPAD_BUTTON_DPAD_LEFT)
        || input.GetGamepadAxisValue(MONA_JOYSTICK_1, MONA_GAMEPAD_AXIS_LEFT_X) < -0.25f)
    {
        m_transform->Translate(glm::vec3(-m_paddleVelocity * timeStep, 0.0f, 0.0f));
    }
    else if (input.IsKeyPressed(MONA_KEY_D) || input.IsKeyPressed(MONA_KEY_RIGHT) 
        || input.IsGamepadButtonPressed(MONA_JOYSTICK_1, MONA_GAMEPAD_BUTTON_DPAD_RIGHT)
        || input.GetGamepadAxisValue(MONA_JOYSTICK_1, MONA_GAMEPAD_AXIS_LEFT_X) > 0.25f)
    {
        m_transform->Translate(glm::vec3(m_paddleVelocity * timeStep, 0.0f, 0.0f));
    }

    if (input.IsKeyPressed(MONA_KEY_ESCAPE) || input.IsGamepadButtonPressed(MONA_JOYSTICK_1, MONA_GAMEPAD_BUTTON_START)) {
        exit(EXIT_SUCCESS);
    }

    if (m_transform->GetLocalTranslation().x < -16.0f) {
        m_transform->SetTranslation(glm::vec3(-16.0f, 0, 0));
    }
    else if (m_transform->GetLocalTranslation().x > 16.0f) {
        m_transform->SetTranslation(glm::vec3(16.0f, 0, 0));
    }

}

Mona::TransformHandle Paddle::GetPaddleTransform() const noexcept {
    return m_transform;
}