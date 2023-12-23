#include <Renderer/Mesh.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Renderer/Stats.hpp>

namespace VaultRenderer {
    Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) : vertices(vertices), indices(indices), material{glm::vec4(1, 1, 1, 1)} {
        // VAO setup
        glGenVertexArrays(1, &VAO);

        // VBO Setup
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(0);
        // example to add more parameters
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texUV));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);

        // EBO Setup

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void Mesh::Draw(Shader &shader) {
        material.BindToShader(shader);
        Statistics::DrawCall();

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<uint32_t>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    Material::Material(const glm::vec4 &color) : color(color) {}

    void Material::SetDiffuse(const std::string &texture_path) {
        diffuse = std::make_unique<Texture>(texture_path, TEXTURE_DIFFUSE);
    }
    void Material::SetSpecular(const std::string &texture_path) {
        specular = std::make_unique<Texture>(texture_path, TEXTURE_SPECULAR);
    }
    void Material::SetNormal(const std::string &texture_path) {
        normal = std::make_unique<Texture>(texture_path, TEXTURE_NORMAL);
    }
    void Material::SetHeight(const std::string &texture_path) {
        height = std::make_unique<Texture>(texture_path, TEXTURE_HEIGHT);
    }

    void Material::BindToShader(Shader &shader) {
        shader.Bind();
        for (uint32_t i = 0; i < 4; i++) {
            // unbind all texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        shader.SetUniform4f("baseColor", color.r, color.g, color.b, color.a);

        if (diffuse) {
            diffuse->Bind(0);
            shader.SetUniform1i("texture_diffuse.tex", 0);
            shader.SetUniform1i("texture_diffuse.defined", 1);
        } else {
            shader.SetUniform1i("texture_diffuse.tex", -1);
            shader.SetUniform1i("texture_diffuse.defined", 0);
        }

        if (specular) {
            specular->Bind(1);
            shader.SetUniform1i("texture_specular.tex", 1);
            shader.SetUniform1i("texture_specular.defined", 1);
        } else {
            shader.SetUniform1i("texture_specular.tex", -1);
            shader.SetUniform1i("texture_specular.defined", 0);
        }

        if (normal) {
            normal->Bind(2);
            shader.SetUniform1i("texture_normal.tex", 2);
            shader.SetUniform1i("texture_normal.defined", 1);
        } else {
            shader.SetUniform1i("texture_normal.tex", -1);
            shader.SetUniform1i("texture_normal.defined", 0);
        }

        if (height) {
            height->Bind(3);
            shader.SetUniform1i("texture_height.tex", 3);
            shader.SetUniform1i("texture_height.defined", 1);
        } else {
            shader.SetUniform1i("texture_height.tex", -1);
            shader.SetUniform1i("texture_height.defined", 0);
        }
    }
} // namespace VaultRenderer