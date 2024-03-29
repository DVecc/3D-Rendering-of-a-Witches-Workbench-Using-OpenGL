// STL
#include <vector>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Project
#include "cylinder.h"

namespace static_meshes_3D {

    Cylinder::Cylinder(float radius, int numSlices, float height, glm::vec4 color, bool withPositions, bool withTextureCoordinates, GLuint textureID, GLuint specularID)
        : StaticMesh3D(withPositions, withTextureCoordinates, true)
        , _radius(radius)
        , _numSlices(numSlices)
        , _height(height)
        , _color(color)
        , _textureID(textureID)
        , _specularID(specularID)
    {
        initializeData();
    }

    float Cylinder::getRadius() const
    {
        return _radius;
    }

    int Cylinder::getSlices() const
    {
        return _numSlices;
    }

    float Cylinder::getHeight() const
    {
        return _height;
    }

    glm::vec4 Cylinder::getColor() const
    {
        return _color;
    }

    void Cylinder::initializeData()
    {
        if (_isInitialized) {
            return;
        }

        // Calculate and cache numbers of vertices
        _numVerticesSide = (_numSlices + 1) * 2;
        _numVerticesTopBottom = _numSlices + 2;
        _numVerticesTotal = _numVerticesSide + _numVerticesTopBottom * 2;

        // Generate VAO and VBO for vertex attributes
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        _vbo.createVBO(getVertexByteSize() * _numVerticesTotal);

        // Pre-calculate sines / cosines for given number of slices
        const auto sliceAngleStep = 2.0f * glm::pi<float>() / static_cast<float>(_numSlices);
        auto currentSliceAngle = 0.0f;
        std::vector<float> sines, cosines;
        for (auto i = 0; i <= _numSlices; i++)
        {
            sines.push_back(sin(currentSliceAngle));
            cosines.push_back(cos(currentSliceAngle));

            // Update slice angle
            currentSliceAngle += sliceAngleStep;
        }

        if (hasPositions())
        {
            // Pre-calculate X and Z coordinates
            std::vector<float> x;
            std::vector<float> z;
            for (auto i = 0; i <= _numSlices; i++)
            {
                x.push_back(cosines[i] * _radius);
                z.push_back(sines[i] * _radius);
            }

            // Add cylinder side vertices
            for (auto i = 0; i <= _numSlices; i++)
            {
                const auto topPosition = glm::vec3(x[i], _height / 2.0f, z[i]);
                const auto bottomPosition = glm::vec3(x[i], -_height / 2.0f, z[i]);
                _vbo.addRawData(&topPosition, sizeof(glm::vec3));
                _vbo.addRawData(&bottomPosition, sizeof(glm::vec3));
            }

            // Add top cylinder cover
            glm::vec3 topCenterPosition(0.0f, _height / 2.0f, 0.0f);
            _vbo.addRawData(&topCenterPosition, sizeof(glm::vec3));
            for (auto i = 0; i <= _numSlices; i++)
            {
                const auto topPosition = glm::vec3(x[i], _height / 2.0f, z[i]);
                _vbo.addRawData(&topPosition, sizeof(glm::vec3));
            }

            // Add bottom cylinder cover
            glm::vec3 bottomCenterPosition(0.0f, -_height / 2.0f, 0.0f);
            _vbo.addRawData(&bottomCenterPosition, sizeof(glm::vec3));
            for (auto i = 0; i <= _numSlices; i++)
            {
                const auto bottomPosition = glm::vec3(x[i], -_height / 2.0f, -z[i]);
                _vbo.addRawData(&bottomPosition, sizeof(glm::vec3));
            }
        }

        if (hasTextureCoordinates())
        {
            // Pre-calculate step size in texture coordinate U
            // I have decided to map the texture twice around cylinder, looks fine
            const auto sliceTextureStepU = 2.0f / static_cast<float>(_numSlices);

            auto currentSliceTexCoordU = 0.0f;
            for (auto i = 0; i <= _numSlices; i++)
            {
                _vbo.addData(glm::vec2(currentSliceTexCoordU, 4.0f));
                _vbo.addData(glm::vec2(currentSliceTexCoordU, 0.0f));

                // Update texture coordinate of current slice 
                currentSliceTexCoordU += sliceTextureStepU;
            }

            // Generate circle texture coordinates for cylinder top cover
            glm::vec2 topBottomCenterTexCoord(0.5f, 0.5f);
            _vbo.addData(topBottomCenterTexCoord);
            for (auto i = 0; i <= _numSlices; i++) {
                _vbo.addData(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y + cosines[i] * 0.5f));
            }

            // Generate circle texture coordinates for cylinder bottom cover
            _vbo.addData(topBottomCenterTexCoord);
            for (auto i = 0; i <= _numSlices; i++) {
                _vbo.addData(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y - cosines[i] * 0.5f));
            }
        }

        // Add color for every vertex of cylinder bottom cover
        _vbo.addData(_color, _numVerticesTotal);

        // Finally upload data to the GPU
        _vbo.bindVBO();
        _vbo.uploadDataToGPU(GL_STATIC_DRAW);
        setVertexAttributesPointers(_numVerticesTotal);

        _isInitialized = true;
    }

    void Cylinder::render() const
    {
        if (!_isInitialized) {
            return;
        }

        glBindVertexArray(_vao);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _specularID);

        // Render cylinder side first
        glDrawArrays(GL_TRIANGLE_STRIP, 0, _numVerticesSide);

        // Render top cover
        glDrawArrays(GL_TRIANGLE_FAN, _numVerticesSide, _numVerticesTopBottom);

        // Render bottom cover
        glDrawArrays(GL_TRIANGLE_FAN, _numVerticesSide + _numVerticesTopBottom, _numVerticesTopBottom);
    }

    void Cylinder::renderPoints() const
    {
        if (!_isInitialized) {
            return;
        }

        // Just render all points as they are stored in the VBO
        glBindVertexArray(_vao);
        glDrawArrays(GL_POINTS, 0, _numVerticesTotal);
    }

} // namespace static_meshes_3D