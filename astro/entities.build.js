import path from "path";
import fs from "fs";
import entities from "./entities.meta.js";

/**
 * @param {string} entity_type
 */
function convertTypeToName(entity_type) {
  const parts = entity_type.split("_");

  return parts
    .map(part => part[0].toUpperCase() + part.slice(1).toLowerCase())
    .join(" ");
}

function createDefaultBehaviorFileContents(entity) {
  const name = convertTypeToName(entity.type).replace(/ /g, "");
  const meshes = entity.meshes || [];
  const placeholderMeshName = meshes.find(name => name.includes("placeholder"));
  const nonPlaceholderMeshNames = meshes.filter(name => !name.includes("placeholder"));

  return `
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior ${name} {
    addMeshes() {
      meshes.${placeholderMeshName} = CUBE(500);
    }

    getMeshes() {
      return_meshes({
        ${nonPlaceholderMeshNames.join(",\n        ")}
      });
    }

    getPlaceholderMesh() {
      return meshes.${placeholderMeshName};
    }

    timeEvolve() {
      // @todo
    }
  };
}
  `;
}

function createFloatString(value) {
  const floatString = value.toString();

  if (floatString.includes(".")) {
    return `${floatString}f`;
  } else {
    return `${floatString}.f`;
  }
}

function createVec3fString(values) {
  const floatValues = values.map(createFloatString);

  return `tVec3f( ${floatValues.join(", ")} )`;
}

function createEntityDefaults(entity) {
  return `
    { ${entity.type}, {
      .name = "${convertTypeToName(entity.type)}",
      .scale = ${createVec3fString(entity.scale)},
      .tint = ${createVec3fString(entity.color)}
    },
  `;
}

function createEntityDefaultsMapString(entities) {
  return entities.map(createEntityDefaults).join("\n");
}

// Build step
{
  const entitiesSourceFilePath = path.resolve("./astro/entities.h");
  const entitiesSourceFileContents = fs.readFileSync(entitiesSourceFilePath).toString();
  const entityDefaultsMapString = createEntityDefaultsMapString(entities);

  // console.log(entitiesSourceFileContents);
  // console.log(entityDefaultsMapString);

  for (const entity of entities) {
    const name = convertTypeToName(entity.type);
    const filename = name.replace(/ /g, "") + ".h";
    const modelsFolderPath = path.resolve(`./astro/3d_models/${entity.type.toLowerCase()}`);
    const behaviorFilePath = path.resolve(`./astro/entity_behaviors/${filename}`);

    if (!fs.existsSync(behaviorFilePath) && !fs.existsSync(modelsFolderPath)) {
      // @temporary
      console.log(`Registering entity: ${entity.type} (${name})`);

      // const behaviorFileContents = createDefaultBehaviorFileContents(entity);

      // fs.mkdirSync(modelsFolderPath);
      // fs.writeFileSync(behaviorFilePath, behaviorFileContents);
    }
  }
}