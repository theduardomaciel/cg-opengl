#include "render/Material.h"
#include <algorithm>
#include <iostream>

namespace cg
{

    Material::Material(const std::string &name)
        : mName(name), mType(MaterialType::OPAQUE)
    {
    }

    Material::Material(MaterialType type, const std::string &name)
        : mName(name), mType(type)
    {
        // Ajusta propriedades baseado no tipo
        switch (type)
        {
        case MaterialType::TRANSPARENT:
            mAlpha = 0.5f;
            mIndexOfRefraction = 1.5f; // Vidro comum
            break;
        case MaterialType::EMISSIVE:
            mEmissive = glm::vec3(1.0f, 1.0f, 1.0f);
            break;
        case MaterialType::OPAQUE:
        default:
            // Valores padrão já definidos
            break;
        }
    }

    void Material::setAlpha(float alpha)
    {
        mAlpha = std::clamp(alpha, 0.0f, 1.0f);

        // Atualiza tipo automaticamente baseado na transparência
        if (mAlpha < 1.0f && mType == MaterialType::OPAQUE)
        {
            mType = MaterialType::TRANSPARENT;
        }
        else if (mAlpha >= 1.0f && mType == MaterialType::TRANSPARENT)
        {
            mType = MaterialType::OPAQUE;
        }
    }

    Material Material::createGlass(float alpha, const glm::vec3 &tint)
    {
        Material glass(MaterialType::TRANSPARENT, "Glass Material");

        glass.setAlbedo(tint);
        glass.setAlpha(alpha);
        glass.setSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
        glass.setShininess(128.0f);        // Vidro tem reflexo bem definido
        glass.setIndexOfRefraction(1.52f); // Vidro comum

        std::cout << "Material de vidro criado: alpha=" << alpha << ", tint=("
                  << tint.x << ", " << tint.y << ", " << tint.z << ")" << std::endl;

        return glass;
    }

    Material Material::createMetal(const glm::vec3 &color, float shininess)
    {
        Material metal(MaterialType::OPAQUE, "Metal Material");

        metal.setAlbedo(color * 0.3f); // Metais têm baixa reflexão difusa
        metal.setSpecular(color);      // Alta reflexão especular
        metal.setShininess(shininess);

        return metal;
    }

    Material Material::createPlastic(const glm::vec3 &color)
    {
        Material plastic(MaterialType::OPAQUE, "Plastic Material");

        plastic.setAlbedo(color);
        plastic.setSpecular(glm::vec3(0.5f, 0.5f, 0.5f)); // Reflexão especular moderada
        plastic.setShininess(16.0f);                      // Brilho mais difuso que metal

        return plastic;
    }

} // namespace cg
