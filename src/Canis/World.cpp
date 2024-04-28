#include "World.hpp"
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace Canis
{
    World::World(Window *_window, InputManager *_inputManager)
    {
        m_window = _window;
        m_inputManager = _inputManager;
    }

    void World::Update(double _deltaTime)
    {
        UpdateCameraMovement(_deltaTime);

        for (int i = 0; i < m_entities.size(); i++)
        {
            if (m_entities[i].Update != nullptr)
            {
                m_entities[i].Update(*this, m_entities[i], 0.1f);
            }
        }
    }

    void World::Draw(double _deltaTime)
    {
        mat4 project = mat4(1.0f);
        project = perspective(radians(45.0f),
                              (float)m_window->GetScreenWidth() / (float)m_window->GetScreenHeight(),
                              0.01f, 100.0f);

        for (int i = 0; i < m_entities.size(); i++)
        {
            if (m_entities[i].active == false)
                continue;
            
            Shader *shader = m_entities[i].shader;
            shader->Use();
            shader->SetVec3("COLOR", 1.0f, 1.0f, 1.0f);
            shader->SetVec3("VIEWPOS", m_camera.Position);
            shader->SetInt("NUMBEROFPOINTLIGHTS", 4);
            shader->SetFloat("TIME", SDL_GetTicks() / 1000.0f);

            UpdateLights(*shader);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_entities[i].albedo->id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_entities[i].specular->id);

            shader->SetMat4("VIEW", m_camera.GetViewMatrix());
            shader->SetMat4("PROJECTION", project);

            shader->SetMat4("TRANSFORM", m_entities[i].transform.Matrix());
            Canis::Draw(*m_entities[i].model);
            shader->UnUse();
        }
    }

    void World::Spawn(Entity _entity)
    {
        m_entities.push_back(_entity);
    }

    void World::SpawnPointLight(PointLight _light)
    {
        m_pointLights.push_back(_light);
    }

    void World::SpawnDirectionalLight(DirectionalLight _light)
    {
        m_directionalLight = _light;
    }

    Entity *World::GetEntityWithTag(std::string _tag)
    {
        for (int i = 0; i < m_entities.size(); i++)
        {
            if (m_entities[i].tag == _tag)
            {
                return &m_entities[i];
            }
        }

        return nullptr;
    }

    std::vector<Entity *> World::GetEntitiesWithTag(std::string _tag)
    {
        std::vector<Entity *> matches = {};

        for (int i = 0; i < m_entities.size(); i++)
        {
            if (m_entities[i].tag == _tag)
            {
                matches.push_back(&m_entities[i]);
            }
        }

        return matches;
    }

    // returns nullptr when light is not found
    PointLight* World::GetPointLight(glm::vec3 _position)
    {
        for(int i = 0; i < m_pointLights.size(); i++)
            if (m_pointLights[i].position == _position)
                return &m_pointLights[i];
        
        return nullptr;
    }

    void World::UpdateLights(Canis::Shader &_shader)
    {
        _shader.SetVec3("DIRECTIONALLIGHT.direction", m_directionalLight.direction);
        _shader.SetVec3("DIRECTIONALLIGHT.ambient", m_directionalLight.ambient);
        _shader.SetVec3("DIRECTIONALLIGHT.diffuse", m_directionalLight.diffuse);
        _shader.SetVec3("DIRECTIONALLIGHT.specular", m_directionalLight.specular);

        for (int i = 0; i < m_pointLights.size(); i++)
        {
            _shader.SetVec3("POINTLIGHTS[" + std::to_string(i) + "].position", m_pointLights[i].position);
            _shader.SetVec3("POINTLIGHTS[" + std::to_string(i) + "].ambient", m_pointLights[i].ambient);
            _shader.SetVec3("POINTLIGHTS[" + std::to_string(i) + "].diffuse", m_pointLights[i].diffuse);
            _shader.SetVec3("POINTLIGHTS[" + std::to_string(i) + "].specular", m_pointLights[i].specular);
            _shader.SetFloat("POINTLIGHTS[" + std::to_string(i) + "].constant", m_pointLights[i].constant);
            _shader.SetFloat("POINTLIGHTS[" + std::to_string(i) + "].linear", m_pointLights[i].linear);
            _shader.SetFloat("POINTLIGHTS[" + std::to_string(i) + "].quadratic", m_pointLights[i].quadratic);
        }
    }

    void World::UpdateCameraMovement(double _deltaTime)
    {
        if (m_inputManager->GetKey(SDL_SCANCODE_W))
            m_camera.ProcessKeyboard(Canis::Camera_Movement::FORWARD, _deltaTime);

        if (m_inputManager->GetKey(SDL_SCANCODE_S))
            m_camera.ProcessKeyboard(Canis::Camera_Movement::BACKWARD, _deltaTime);

        if (m_inputManager->GetKey(SDL_SCANCODE_A))
            m_camera.ProcessKeyboard(Canis::Camera_Movement::LEFT, _deltaTime);

        if (m_inputManager->GetKey(SDL_SCANCODE_D))
            m_camera.ProcessKeyboard(Canis::Camera_Movement::RIGHT, _deltaTime);

        if (m_window->GetMouseLock())
            m_camera.ProcessMouseMovement(m_inputManager->mouseRel.x, -m_inputManager->mouseRel.y, true);

        if (m_inputManager->JustPressedKey(SDLK_ESCAPE))
            m_window->MouseLock(!m_window->GetMouseLock());
    }
}