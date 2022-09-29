- [x] my_vulkanRHI.h  
rendering hardware interface

- [ ] my_descriptor.h
- [ ] my_vulkanUtil.h
- [x] my_commandbuffer.h
- [x] my_gui.h  

- [ ] my_pipeline.h
  - [x] scene pipeline
  - [x] shadowmap pipeline
  - [ ] compute pipeline  

- [ ] my_renderer.h
- [ ] my_model.h 
    - [ ] load model
    - [ ] add texture
    - [ ] add transform data like ubo etc.

- [ ] physics system
- [ ] animation system
- [ ] scene managment

- [x] my_texture.h
  - [x] texture pool
  - [x] correspondence with descriptor
- [x] my_shader.h
- [ ] my_scene.h
- [x] my_vertex.h 
  - [x] class MyVertex_Default
- [x] my_event.h 
- [ ] my_descriptor.h
- [ ] my_app.h
  - run() 
    - void prepare() 
      - virtual void createDescriptorSets()=0;
      - virtual void createPipelines()=0;
      - virtual void prepareData()=0;
      - gui.init()
    - void mainloop()
      - virtual void updateData()=0;
      - void recordCommands()
        - virtual void renderpassCommands(uint32_t imageindex)=0;
    - virtual void cleanup()


想把场景中所有mesh数据都放在相同的vertexbuffer和indexbuffer中，然后一个drawIndex命令画完，但是不同的mesh对应不同的texture，怎么解决给不同mesh分配不同texture的问题比较好啊，把textureID放在vertex数据中吗？