
/*
 * Copyright 2011-2013 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "render/mesh.h"
#include "render/object.h"

#include "blender/blender_sync.h"
#include "blender/blender_util.h"

CCL_NAMESPACE_BEGIN

Mesh *BlenderSync::sync_geometry(BL::Depsgraph &b_depsgraph,
                                 BL::Object &b_ob,
                                 BL::Object &b_ob_instance,
                                 bool object_updated,
                                 bool use_particle_hair)
{
  /* Test if we can instance or if the object is modified. */
  BL::ID b_ob_data = b_ob.data();
  BL::ID b_key_id = (BKE_object_is_modified(b_ob)) ? b_ob_instance : b_ob_data;
  MeshKey key(b_key_id.ptr.data, use_particle_hair);
  BL::Material material_override = view_layer.material_override;
  Shader *default_shader = scene->default_surface;

  /* Find shader indices. */
  vector<Shader *> used_shaders;

  BL::Object::material_slots_iterator slot;
  for (b_ob.material_slots.begin(slot); slot != b_ob.material_slots.end(); ++slot) {
    if (material_override) {
      find_shader(material_override, used_shaders, default_shader);
    }
    else {
      BL::ID b_material(slot->material());
      find_shader(b_material, used_shaders, default_shader);
    }
  }

  if (used_shaders.size() == 0) {
    if (material_override)
      find_shader(material_override, used_shaders, default_shader);
    else
      used_shaders.push_back(default_shader);
  }

  /* Test if we need to sync. */
  Mesh *mesh;

  if (!mesh_map.sync(&mesh, b_key_id, key)) {
    /* If transform was applied to mesh, need full update. */
    if (object_updated && mesh->transform_applied)
      ;
    /* Test if shaders changed, these can be object level so mesh
     * does not get tagged for recalc. */
    else if (mesh->used_shaders != used_shaders)
      ;
    else {
      /* Even if not tagged for recalc, we may need to sync anyway
       * because the shader needs different mesh attributes. */
      bool attribute_recalc = false;

      foreach (Shader *shader, mesh->used_shaders)
        if (shader->need_update_mesh)
          attribute_recalc = true;

      if (!attribute_recalc)
        return mesh;
    }
  }

  /* Ensure we only sync instanced meshes once. */
  if (mesh_synced.find(mesh) != mesh_synced.end())
    return mesh;

  progress.set_sync_status("Synchronizing object", b_ob.name());

  mesh_synced.insert(mesh);

  mesh->clear();
  mesh->used_shaders = used_shaders;
  mesh->name = ustring(b_ob_data.name().c_str());

  if (use_particle_hair) {
    sync_hair(b_depsgraph, b_ob, mesh);
  }
  else if (object_fluid_gas_domain_find(b_ob)) {
    sync_volume(b_ob, mesh);
  }
  else {
    sync_mesh(b_depsgraph, b_ob, mesh);
  }

  return mesh;
}

void BlenderSync::sync_geometry_motion(BL::Depsgraph &b_depsgraph,
                                       BL::Object &b_ob,
                                       Object *object,
                                       float motion_time,
                                       bool use_particle_hair)
{
  /* Ensure we only sync instanced meshes once. */
  Mesh *mesh = object->mesh;

  if (mesh_motion_synced.find(mesh) != mesh_motion_synced.end())
    return;

  mesh_motion_synced.insert(mesh);

  /* Ensure we only motion sync meshes that also had mesh synced, to avoid
   * unnecessary work and to ensure that its attributes were clear. */
  if (mesh_synced.find(mesh) == mesh_synced.end())
    return;

  /* Find time matching motion step required by mesh. */
  int motion_step = mesh->motion_step(motion_time);
  if (motion_step < 0) {
    return;
  }

  if (use_particle_hair) {
    sync_hair_motion(b_depsgraph, b_ob, mesh, motion_step);
  }
  else if (object_fluid_gas_domain_find(b_ob)) {
    /* No volume motion blur support yet. */
  }
  else {
    sync_mesh_motion(b_depsgraph, b_ob, mesh, motion_step);
  }
}

CCL_NAMESPACE_END