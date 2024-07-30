# ArcFramework


https://marcelbraghetto.github.io/a-simple-triangle/2019/09/21/part-25/
https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer


# TODO:

Fix Ownership model:

The problem is that they need to know forward but own in reverse:
ownership: device -> renderer -> renderpipeline
knowlegde: device <- renderer <- renderpipeline

So what happens is they deallocate themselves even though they are needed in deallocation of their members.



