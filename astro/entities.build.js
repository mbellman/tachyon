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

  return `tVec3f(${floatValues.join(", ")})`;
}

function createEntityTypeEnumValues(entities) {
  return `
    UNSPECIFIED = -1,
    ${entities.map(entity => entity.type).join(",\n    ")}
  `;
}

function createEntityTypesListValues(entities) {
  const sortedTypes = [...entities].sort((a, b) => a.type < b.type ? -1 : 1);

  return `
    ${sortedTypes.map(entity => entity.type).join(",\n    ")}
  `;
}

function createEntityMeshesString(entities) {
  // Indentation/formatting here is awkward to read,
  // but is intentional in how it generates output
  return `
    uint16
    ${entities.map(entity => `
      // ${entity.type}
      ${entity.meshes.join(",\n      ")}`)
    .join(",\n      ")}
    ;
  `;
}

function createEntityArraysString(entities) {
  return "\n" + entities.map(entity => {
    return `    EntityList ${entity.list};`
  }).join("\n") + "\n  ";
}

function createEntityDefaultsString(entity) {
  return `
    { ${entity.type}, {
      .name = "${convertTypeToName(entity.type)}",
      .scale = ${createVec3fString(entity.scale)},
      .tint = ${createVec3fString(entity.color)}
    } },
  `;
}

function createEntityDefaultsMapString(entities) {
  return entities.map(createEntityDefaultsString).join("");
}

function replaceFileContents(fileContents, startSpecifier, replacementText, endSpecifier) {
  const startPosition = fileContents.indexOf(startSpecifier) + startSpecifier.length;
  const endPosition = fileContents.indexOf(endSpecifier, startPosition);

  const head = fileContents.substring(0, startPosition);
  const tail = fileContents.substring(endPosition);

  return head + replacementText + tail;
}

function addEntityTypesEnum(entitiesFileContents, entities) {
  return replaceFileContents(
    entitiesFileContents,
    "enum EntityType {",
    createEntityTypeEnumValues(entities),
    "};"
  );
}

function addEntityTypesList(entitiesFileContents, entities) {
  return replaceFileContents(
    entitiesFileContents,
    "static std::vector<EntityType> entity_types = {",
    createEntityTypesListValues(entities),
    "};"
  );
}

function addEntityMeshes(entitiesFileContents, entities) {
  return replaceFileContents(
    entitiesFileContents,
    "struct EntityMeshIds {",
    createEntityMeshesString(entities),
    "};"
  );
}

function addEntityArrays(entitiesFileContents, entities) {
  return replaceFileContents(
    entitiesFileContents,
    "struct EntityContainers {",
    createEntityArraysString(entities),
    "};"
  );
}

function addEntityDefaultsMap(entitiesFileContents, entities) {
  return replaceFileContents(
    entitiesFileContents,
    "static std::map<EntityType, EntityDefaults> entity_defaults_map = {",
    createEntityDefaultsMapString(entities),
    "};"
  );
}

function addEntityBehaviorIncludes(dispatcherFileContents, entities) {
  const includePaths = entities.map(entity => {
    const name = convertTypeToName(entity.type);
    const filename = name.replace(/ /g, "") + ".h";

    return `astro/entity_behaviors/${filename}`;
  });

  return replaceFileContents(
    dispatcherFileContents,
    "#include \"astro/entity_dispatcher.h\"",
    `\n\n${includePaths.map(includePath => `#include "${includePath}"`).join("\n")}\n\n`,
    "using namespace astro;"
  );
}

// Build step
{
  const entitiesFilePath = path.resolve("./astro/entities.h");
  const dispatcherFilePath = path.resolve("./astro/entity_dispatcher.cpp");
  const sortedEntities = entities.sort((a, b) => a.type < b.type ? -1 : 1);

  let entitiesFileContents = fs.readFileSync(entitiesFilePath).toString();
  entitiesFileContents = addEntityTypesEnum(entitiesFileContents, sortedEntities);
  entitiesFileContents = addEntityTypesList(entitiesFileContents, sortedEntities);
  entitiesFileContents = addEntityMeshes(entitiesFileContents, sortedEntities);
  entitiesFileContents = addEntityArrays(entitiesFileContents, sortedEntities);
  entitiesFileContents = addEntityDefaultsMap(entitiesFileContents, sortedEntities);

  let dispatcherFileContents = fs.readFileSync(dispatcherFilePath).toString();
  dispatcherFileContents = addEntityBehaviorIncludes(dispatcherFileContents, entities);

  fs.writeFileSync(entitiesFilePath, entitiesFileContents);
  fs.writeFileSync(dispatcherFilePath, dispatcherFileContents);

  for (const entity of sortedEntities) {
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