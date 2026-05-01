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

// Build step
{
  for (const entity of entities) {
    const name = convertTypeToName(entity.type);
    const filename = name.replace(/ /g, "") + ".h";
    const filepath = path.resolve(`./astro/entity_behaviors/${filename}`);

    if (!fs.existsSync(filepath)) {
      // @temporary
      console.log("Doesn't exist!", entity.type, name);

      // @todo
    }
  }
}