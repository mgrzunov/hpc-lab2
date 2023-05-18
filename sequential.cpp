#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>

using namespace std::chrono;

static const high_resolution_clock::time_point total_start_time = high_resolution_clock::now(); 
static high_resolution_clock::time_point start_time;
static high_resolution_clock::time_point stop_time;

#define MEASURE_START() (start_time = high_resolution_clock::now())
#define MEASURE_STOP()  \
    do { \
        stop_time = high_resolution_clock::now(); \
    } while (0)
#define MEASURE_PRINT(tag) (printf("[" tag "] Time = %lld ms\n", duration_cast<milliseconds>(stop_time - start_time).count()))
#define MEASURE_PRINT_TOTAL() (printf("[TOTAL] Time = %lld ms\n", duration_cast<milliseconds>(high_resolution_clock::now() - total_start_time).count()))

#define PIXEL_WIDTH 3840
#define PIXEL_HEIGHT 2160
#define PIXEL_NUM (PIXEL_HEIGHT * PIXEL_WIDTH)
#define FRAME_NUM 60
#define COMPONENT_NUM 3
#define FRAME_SIZE (COMPONENT_NUM * PIXEL_NUM)
#define VIDEO_SIZE (FRAME_SIZE * FRAME_NUM)

#define UNDERSAMPLED_FRAME_SIZE (PIXEL_NUM + PIXEL_NUM / 2)

#define FILE_NAME_WIDTH 512

typedef unsigned int uint;

int main(int argc, char *argv[])
{
    // Rudimentary startup arg check
    if (argc != 2)
    {
        fprintf(stderr, "Should have 1 argument, the original video path!\n");
        return -1;
    }

    /************************/
    /*** OPEN VIDEO FILES ***/
    /************************/
    FILE *rgb_video_file = fopen(argv[1], "rb");
    if (rgb_video_file == NULL)
    {
        perror("Failed opening rgb video file");
        return -1;
    }

    FILE *yuv_file = fopen("yuv_video.yuv", "wb");
    if (yuv_file == NULL)
    {
        perror("Failed opening yuv output file");
        return -1;
    }

    FILE *yuv_undersampled_file = fopen("yuv_undersampled_video.yuv", "wb");
    if (yuv_undersampled_file == NULL)
    {
        perror("Failed opening yuv output file");
        return -1;
    }

    FILE *yuv_oversampled_file = fopen("yuv_oversampled_video.yuv", "wb");
    if (yuv_oversampled_file == NULL)
    {
        perror("Failed opening yuv output file");
        return -1;
    }

    // Program uses only 2 frame buffers
    uint8_t *video_data_1 = (uint8_t *)malloc(sizeof(*video_data_1) * VIDEO_SIZE);
    uint8_t *video_data_2 = (uint8_t *)malloc(sizeof(*video_data_1) * VIDEO_SIZE);

    size_t byte_count;
    uint8_t *rgb_video_data = &video_data_1[0];
    uint8_t *yuv_video_data = &video_data_1[0]; // Can use same buffer
    uint8_t *yuv_undersampled_data = &video_data_2[0]; // Switches buffer
    uint8_t *yuv_oversampled_data = &video_data_1[0]; // Switches buffer

    /**********************/
    /*** LOAD RGB VIDEO ***/
    /**********************/
    MEASURE_START();
    byte_count = fread(
        &rgb_video_data[0],
        sizeof(video_data_1[0]), VIDEO_SIZE, 
        rgb_video_file
    );
    if (byte_count != VIDEO_SIZE)
    {
        fprintf(stderr, "Only loaded %u bytes of original!\n", (uint)byte_count);
        return -1;
    }
    MEASURE_STOP();
    //MEASURE_PRINT("Load RGB"); Measuring for total

    /*******************/
    /*** RGB --> YUV ***/
    /*******************/
    MEASURE_START();
    for (uint frame_i = 0; frame_i != FRAME_NUM; ++frame_i)
    {
        for (uint pixel_i = 0; pixel_i != PIXEL_NUM; ++pixel_i)
        {
            const uint8_t r_value = rgb_video_data[frame_i * FRAME_SIZE + pixel_i];
            const uint8_t g_value = rgb_video_data[frame_i * FRAME_SIZE + pixel_i + PIXEL_NUM];
            const uint8_t b_value = rgb_video_data[frame_i * FRAME_SIZE + pixel_i + PIXEL_NUM * 2];

            const uint8_t y_value = (uint8_t)(0.257f * r_value + 0.504f * g_value + 0.098f * b_value + 16);
            const uint8_t u_value = (uint8_t)(-0.148f * r_value - 0.291f * g_value + 0.439f * b_value + 128);
            const uint8_t v_value = (uint8_t)(0.439f * r_value - 0.368f * g_value - 0.071 * b_value + 128);

            yuv_video_data[frame_i * FRAME_SIZE + pixel_i]                 = y_value;
            yuv_video_data[frame_i * FRAME_SIZE + pixel_i + PIXEL_NUM]     = u_value;
            yuv_video_data[frame_i * FRAME_SIZE + pixel_i + PIXEL_NUM * 2] = v_value;
        }
    }
    MEASURE_STOP();
    MEASURE_PRINT("RGB to YUV");

    MEASURE_START();
    byte_count = fwrite(
        &yuv_video_data[0],
        sizeof(video_data_1[0]), VIDEO_SIZE, 
        yuv_file
    );
    if (byte_count != VIDEO_SIZE)
    {
        fprintf(stderr, "Only wrote %u bytes of yuv video!\n", (uint)byte_count);
        return -1;
    }
    MEASURE_STOP();
    //MEASURE_PRINT("Store YUV"); Measuring for total time

    /*************************************/
    /*** Undersampling 4:4:4 --> 4:2:0 ***/
    /*************************************/
    MEASURE_START();
    for (uint frame_i = 0; frame_i != FRAME_NUM; ++frame_i)
    {
        // Just copy Y values
        memcpy(
            &yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE], 
            &yuv_video_data[frame_i * FRAME_SIZE], 
            PIXEL_NUM
        );
        for (uint row = 0; row != PIXEL_HEIGHT; row += 2)
        {            
            for (uint col = 0; col != PIXEL_WIDTH; col += 2)
            {
                // Average 4 U values into 1 (+ PIXEL_NUM because Y is preserved)
                yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + PIXEL_NUM] = (
                    yuv_video_data[frame_i * FRAME_SIZE +    row    * PIXEL_WIDTH + col     + PIXEL_NUM] +
                    yuv_video_data[frame_i * FRAME_SIZE +    row    * PIXEL_WIDTH + col + 1 + PIXEL_NUM] +
                    yuv_video_data[frame_i * FRAME_SIZE + (row + 1) * PIXEL_WIDTH + col     + PIXEL_NUM] +
                    yuv_video_data[frame_i * FRAME_SIZE + (row + 1) * PIXEL_WIDTH + col + 1 + PIXEL_NUM]
                ) / 4;
                // Average 4 V values into 1 (+ PIXEL_NUM + PIXEL_NUM / 4 for Y and U offset)
                yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + (PIXEL_NUM + PIXEL_NUM / 4)] = (
                    yuv_video_data[frame_i * FRAME_SIZE +    row    * PIXEL_WIDTH + col     + (2 * PIXEL_NUM)] +
                    yuv_video_data[frame_i * FRAME_SIZE +    row    * PIXEL_WIDTH + col + 1 + (2 * PIXEL_NUM)] +
                    yuv_video_data[frame_i * FRAME_SIZE + (row + 1) * PIXEL_WIDTH + col     + (2 * PIXEL_NUM)] +
                    yuv_video_data[frame_i * FRAME_SIZE + (row + 1) * PIXEL_WIDTH + col + 1 + (2 * PIXEL_NUM)]
                ) / 4;
            }
        }
    }
    MEASURE_STOP();
    MEASURE_PRINT("Undersample YUV");

    MEASURE_START();
    byte_count = fwrite(
        &yuv_undersampled_data[0],
        sizeof(video_data_1[0]), UNDERSAMPLED_FRAME_SIZE * FRAME_NUM, 
        yuv_undersampled_file
    );
    if (byte_count != UNDERSAMPLED_FRAME_SIZE * FRAME_NUM)
    {
        fprintf(stderr, "Only wrote %u bytes of undersampled video!\n", (uint)byte_count);
        return -1;
    }
    MEASURE_STOP();
    //MEASURE_PRINT("Store undersampled"); Measuring for total time

    /*************************************/
    /*** Oversampling  4:2:0 --> 4:4:4 ***/
    /*************************************/
    MEASURE_START();
    for (uint frame_i = 0; frame_i != FRAME_NUM; ++frame_i)
    {
        // Just copy Y values
        memcpy(
            &yuv_oversampled_data[frame_i * FRAME_SIZE], 
            &yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE], 
            PIXEL_NUM
        ); 
        for (uint row = 0; row != PIXEL_HEIGHT; row += 2)
        {
            for (uint col = 0; col != PIXEL_WIDTH; col += 2)
            {
                // Inverse operation of undersampling
                // Oversample U component
                yuv_oversampled_data[frame_i * FRAME_SIZE +    row    * PIXEL_WIDTH + col     + PIXEL_NUM] =
                    yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + PIXEL_NUM]; 
                yuv_oversampled_data[frame_i * FRAME_SIZE +    row    * PIXEL_WIDTH + col + 1 + PIXEL_NUM] =
                    yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + PIXEL_NUM]; 
                yuv_oversampled_data[frame_i * FRAME_SIZE + (row + 1) * PIXEL_WIDTH + col     + PIXEL_NUM] =
                    yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + PIXEL_NUM]; 
                yuv_oversampled_data[frame_i * FRAME_SIZE + (row + 1) * PIXEL_WIDTH + col + 1 + PIXEL_NUM] =
                    yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + PIXEL_NUM]; 

                // Oversample V component
                yuv_oversampled_data[frame_i * FRAME_SIZE +    row    * PIXEL_WIDTH + col     + (2 * PIXEL_NUM)] =
                    yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + (PIXEL_NUM + PIXEL_NUM / 4)]; 
                yuv_oversampled_data[frame_i * FRAME_SIZE +    row    * PIXEL_WIDTH + col + 1 + (2 * PIXEL_NUM)] =
                    yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + (PIXEL_NUM + PIXEL_NUM / 4)];
                yuv_oversampled_data[frame_i * FRAME_SIZE + (row + 1) * PIXEL_WIDTH + col     + (2 * PIXEL_NUM)] =
                    yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + (PIXEL_NUM + PIXEL_NUM / 4)];
                yuv_oversampled_data[frame_i * FRAME_SIZE + (row + 1) * PIXEL_WIDTH + col + 1 + (2 * PIXEL_NUM)] =
                    yuv_undersampled_data[frame_i * UNDERSAMPLED_FRAME_SIZE + (row / 2) * (PIXEL_WIDTH / 2) + (col / 2) + (PIXEL_NUM + PIXEL_NUM / 4)];
            }
        }
    }
    MEASURE_STOP();
    MEASURE_PRINT("Oversample YUV");

    MEASURE_START();
    byte_count = fwrite(
        &yuv_oversampled_data[0],
        sizeof(video_data_1[0]), VIDEO_SIZE, 
        yuv_oversampled_file
    );
    if (byte_count != VIDEO_SIZE)
    {
        fprintf(stderr, "Only wrote %u bytes of oversampled video!\n", (uint)byte_count);
        return -1;
    }
    MEASURE_STOP();
    //MEASURE_PRINT("Store oversampled"); Measuring for total time

    // Print total time
    MEASURE_PRINT_TOTAL();

    /*************************/
    /*** CLOSE VIDEO FILES ***/
    /*************************/
    fclose(rgb_video_file);
    fclose(yuv_file);
    fclose(yuv_undersampled_file);
    fclose(yuv_oversampled_file);

    return 0;
}
