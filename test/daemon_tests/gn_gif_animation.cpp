#include <vector>
#include <map>
#include <stdio.h>
#include <fstream>
#include <assert.h>

#include "hhal.h"
#include "hhal_client.h"

#include "arguments.h"

#include "mango_arguments.h"
#include "event_utils.h"
#include "gn_dummy_rm.h"
#include "AnimatedGifSaver.h"

using namespace hhal;

#define DAEMON_PATH "/tmp/server-test"

#define KERNEL_SCALE_PATH   "gn_kernels/gif_scale_kernel/scale_kernel"
#define KERNEL_COPY_PATH    "gn_kernels/gif_copy_kernel/copy_kernel"
#define KERNEL_SMOOTH_PATH  "gn_kernels/gif_smooth_kernel/smooth_kernel"
#define KSCALE  1
#define KCOPY   2
#define KSMOOTH 3
#define B1 1
#define B2 2
#define B3 3

typedef unsigned char Byte;

// Lets define a few frames for this little demo...

// red and white RGB pixels
#define R 255,0,0
#define W 255,255,255

// ...frames sizes
const int SX=5;
const int SY=7;

// ...and, the frames themselves
// (note: they are defined bottom-to-top (a-la OpenGL) so they appear upside-down).

Byte frame0[SX*SY*3] = {
    W,W,W,W,W,
    W,W,R,W,W,
    W,R,W,R,W,
    W,R,W,R,W,
    W,R,W,R,W,
    W,W,R,W,W,
    W,W,W,W,W,
};

Byte frame1[SX*SY*3] = {
    W,W,W,W,W,
    W,W,R,W,W,
    W,W,R,W,W,
    W,W,R,W,W,
    W,R,R,W,W,
    W,W,R,W,W,
    W,W,W,W,W,
};

Byte frame2[SX*SY*3]= {
    W,W,W,W,W,
    W,R,R,R,W,
    W,R,W,W,W,
    W,W,R,W,W,
    W,W,W,R,W,
    W,R,R,W,W,
    W,W,W,W,W,
};

Byte frame3[SX*SY*3]= {
    W,W,W,W,W,
    W,R,R,W,W,
    W,W,W,R,W,
    W,W,R,W,W,
    W,W,W,R,W,
    W,R,R,W,W,
    W,W,W,W,W,
};

/* These variables and functions are added to support the scale & smooth
 * functionalities.
 */
Byte frame[SX*2*SY*2*3];
Byte sframe[SX*2*SY*2*3];

void double_frame(Byte *out, Byte *in, int X, int Y)
{
    int X2=X*2;
    int Y2=Y*2;
    for(int x=0; x<X2; x++)
        for(int y=0; y<Y2; y++)
            for(int c=0; c<3; c++) {
                out[y*X2*3+x*3+c]=in[y/2*X*3+x/2*3+c];
            }
}

void copy_frame(Byte *out, Byte *in, int X, int Y)
{
    for(int x=0; x<X; x++)
        for(int y=0; y<Y; y++)
            for(int c=0; c<3; c++)
                out[y*X*3+x*3+c] =	in[y*X*3+x*3+c];
}

void smooth_frame(Byte *out, Byte *in, int X, int Y)
{
    for(int x=1; x<X-1; x++)
        for(int y=1; y<Y-1; y++)
            for(int c=0; c<3; c++) {
                out[y*X*3+x*3+c]=
                    (in[y*X*3+x*3+c]+
                     in[y*X*3+(x-1)*3+c]+
                     in[y*X*3+(x+1)*3+c]+
                     in[(y-1)*X*3+x*3+c]+
                     in[(y+1)*X*3+x*3+c]) / 5;
            }
}


// Note: it may be necessary to copy ./gn/gn/config.xml to MANGO_ROOT/usr/local/share/config.xml

int main(void) {
    hhal_daemon::HHALClient hhal(DAEMON_PATH);
    GNManager gn_manager;
    gn_manager.initialize();

    AnimatedGifSaver saver(SX*2,SY*2);

    std::ifstream kernel_scale_fd(KERNEL_SCALE_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_scale_fd.good() && "Scale kernel file does not exist");
    size_t kernel_scale_size = (size_t) kernel_scale_fd.tellg() + 1;
    
    std::ifstream kernel_copy_fd(KERNEL_COPY_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_copy_fd.good() && "Copy kernel does not exist");
    size_t kernel_copy_size = (size_t) kernel_copy_fd.tellg() + 1;

    std::ifstream kernel_smooth_fd(KERNEL_SMOOTH_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_smooth_fd.good() && "Smooth kernel does not exist");
    size_t kernel_smooth_size = (size_t) kernel_smooth_fd.tellg() + 1;

    mango_kernel kernel_scale = { KSCALE, kernel_scale_size };
    gn_rm::registered_kernel r_kernel_scale = gn_rm::register_kernel(kernel_scale);
    mango_kernel kernel_copy = { KCOPY, kernel_copy_size };
    gn_rm::registered_kernel r_kernel_copy = gn_rm::register_kernel(kernel_copy);
    mango_kernel kernel_smooth = { KSMOOTH, kernel_smooth_size };
    gn_rm::registered_kernel r_kernel_smooth = gn_rm::register_kernel(kernel_smooth);

    std::vector<mango_buffer> buffers = {
        {B1, SX*SY*3*sizeof(Byte),      {},                 {KSCALE}},
        {B2, SX*2*SY*2*3*sizeof(Byte),  {KSCALE},           {KCOPY, KSMOOTH}},
        {B3, SX*2*SY*2*3*sizeof(Byte),  {KCOPY, KSMOOTH},   {}},
    };

    std::vector<gn_rm::registered_buffer> r_buffers;
    for(auto &b: buffers) {
        r_buffers.push_back(gn_rm::register_buffer(b));
    }

    mango_event kernel_scale_termination_event = {r_kernel_scale.kernel_termination_event};
    mango_event kernel_copy_termination_event = {r_kernel_copy.kernel_termination_event};
    mango_event kernel_smooth_termination_event = {r_kernel_smooth.kernel_termination_event};

    mango_event sync_ev = {gn_rm::get_new_event_id(), {r_kernel_copy.k.id, r_kernel_smooth.k.id}, {r_kernel_scale.k.id, r_kernel_copy.k.id}};
    std::vector<mango_event> events;
    events.push_back(sync_ev);
    events.push_back({r_kernel_scale.kernel_termination_event, {r_kernel_scale.k.id}, {r_kernel_scale.k.id}});
    events.push_back({r_kernel_smooth.kernel_termination_event, {r_kernel_smooth.k.id}, {r_kernel_smooth.k.id}});
    events.push_back({r_kernel_copy.kernel_termination_event, {r_kernel_copy.k.id}, {r_kernel_copy.k.id}});
    for(int e_id: r_kernel_scale.task_events) {
        events.push_back({e_id, {r_kernel_scale.k.id}, {r_kernel_scale.k.id}});
    }
    for(int e_id: r_kernel_copy.task_events) {
        events.push_back({e_id, {r_kernel_copy.k.id}, {r_kernel_copy.k.id}});
    }
    for(int e_id: r_kernel_smooth.task_events) {
        events.push_back({e_id, {r_kernel_smooth.k.id}, {r_kernel_smooth.k.id}});
    }
    for(auto &b: r_buffers) {
        events.push_back({b.event, b.b.kernels_in, b.b.kernels_out});
    }

    /* resource allocation */
    gn_rm::resource_allocation(hhal, gn_manager, {r_kernel_scale, r_kernel_smooth, r_kernel_copy}, r_buffers, events);

    const std::map<hhal::Unit, std::string> kernel_scale_images =   {{hhal::Unit::GN, KERNEL_SCALE_PATH}};
    const std::map<hhal::Unit, std::string> kernel_copy_images =    {{hhal::Unit::GN, KERNEL_COPY_PATH}};
    const std::map<hhal::Unit, std::string> kernel_smooth_images =  {{hhal::Unit::GN, KERNEL_SMOOTH_PATH}};
    
    hhal.kernel_write(kernel_scale.id, kernel_scale_images);
    hhal.kernel_write(kernel_copy.id, kernel_copy_images);
    hhal.kernel_write(kernel_smooth.id, kernel_smooth_images);
    printf("resource allocation done\n");

    /* Execution preparation */

    int sx = SX;
    int sy = SY;
    int sx2 = SX*2;
    int sy2 = SY*2;

    scalar_arg scalar_arg_sx1 = {hhal::ScalarType::INT, sizeof(int32_t)} ;
    scalar_arg_sx1.aint32 = sx;
    scalar_arg scalar_arg_sy1 = {hhal::ScalarType::INT, sizeof(int32_t)} ;
    scalar_arg_sy1.aint32 = sy;
    scalar_arg scalar_arg_sx2 = {hhal::ScalarType::INT, sizeof(int32_t)} ;
    scalar_arg_sx2.aint32 = sx2;
    scalar_arg scalar_arg_sy2 = {hhal::ScalarType::INT, sizeof(int32_t)} ;
    scalar_arg_sy2.aint32 = sy2;

    Arguments args_k_scale;
    args_k_scale.add_event({sync_ev.id});
    args_k_scale.add_buffer({B2});
    args_k_scale.add_buffer({B1});
    args_k_scale.add_scalar(scalar_arg_sx1);
    args_k_scale.add_scalar(scalar_arg_sy1);

    Arguments args_k_copy;
    args_k_copy.add_event({sync_ev.id});
    args_k_copy.add_buffer({B3});
    args_k_copy.add_buffer({B2});
    args_k_copy.add_scalar(scalar_arg_sx2);
    args_k_copy.add_scalar(scalar_arg_sy2);

    Arguments args_k_smooth;
    args_k_smooth.add_event({sync_ev.id});
    args_k_smooth.add_buffer({B3});
    args_k_smooth.add_buffer({B2});
    args_k_smooth.add_scalar(scalar_arg_sx2);
    args_k_smooth.add_scalar(scalar_arg_sy2);
    
    Byte *frames[4] = { frame0, frame1, frame2, frame3 };

    for(int i=3; i>=0; i--) {
        printf("Running frame %d\n", i);
        /* Data transfer host->device */
        hhal.write_to_memory(B1, frames[i], buffers[0].size);

        /* spawn kernel */

        // Gotta write 0 to the event before starting the kernel
        events::write(hhal, kernel_scale_termination_event.id, 0);
        events::write(hhal, kernel_copy_termination_event.id, 0);
        events::write(hhal, kernel_smooth_termination_event.id, 0);
        
        hhal.kernel_start(KSCALE,   args_k_scale);
        hhal.kernel_start(KCOPY,    args_k_copy);
        hhal.kernel_start(KSMOOTH,  args_k_smooth);

        printf("Waiting for scale kernel termination event\n");
        events::wait(hhal, kernel_scale_termination_event.id, 1);
        printf("Waiting for copy kernel termination event\n");
        events::wait(hhal, kernel_copy_termination_event.id, 1);
        printf("Waiting for smooth kernel termination event\n");
        events::wait(hhal, kernel_smooth_termination_event.id, 1);

        hhal.read_from_memory(B3, sframe, SX*2*SY*2*3);

        saver.AddFrame(sframe, i+1);
    }

    saver.Save("0123_kernel.gif");
    
    gn_rm::resource_deallocation(hhal, gn_manager, {kernel_scale, kernel_smooth, kernel_copy}, buffers, events);

    printf("Gif animation finished! File name: 0123_kernel.gif\n");

    return 0;
}
