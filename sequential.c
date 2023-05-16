#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define PIXEL_WIDTH 3840
#define PIXEL_HEIGHT 2160
#define PIXEL_NUM (PIXEL_HEIGHT * PIXEL_WIDTH)
#define FRAME_NUM 60
#define VIDEO_SIZE (PIXEL_NUM * 3 * 60)

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
    uint8_t *frame_data_1 = malloc(sizeof(*frame_data_1) * PIXEL_NUM * 3);
    uint8_t *frame_data_2 = malloc(sizeof(*frame_data_1) * PIXEL_NUM * 3);

    // Process all 60 frames
    for (uint frame_i = 0; frame_i != FRAME_NUM; ++frame_i)
    {
        size_t byte_count;
        uint8_t *rgb_frame_data = &frame_data_1[0];

        /**********************/
        /*** LOAD RGB FRAME ***/
        /**********************/
        byte_count = fread(
            &rgb_frame_data[0],
            sizeof(frame_data_1[0]), PIXEL_NUM * 3, 
            rgb_video_file
        );
        if (byte_count != PIXEL_NUM * 3)
        {
            fprintf(stderr, "[f%d - 1] Only loaded %lu bytes of original!\n", frame_i, byte_count);
            break;
        }

        /*******************/
        /*** RGB --> YUV ***/
        /*******************/
        uint8_t *yuv_frame_data = &frame_data_1[0]; // Can use same buffer
        for (uint pixel_i = 0; pixel_i != PIXEL_NUM; ++pixel_i)
        {
            const uint8_t r_value = rgb_frame_data[pixel_i];
            const uint8_t g_value = rgb_frame_data[pixel_i + PIXEL_NUM];
            const uint8_t b_value = rgb_frame_data[pixel_i + PIXEL_NUM * 2];

            const uint8_t y_value = (uint8_t)(0.257f * r_value + 0.504f * g_value + 0.098f * b_value + 16);
            const uint8_t u_value = (uint8_t)(-0.148f * r_value - 0.291f * g_value + 0.439f * b_value + 128);
            const uint8_t v_value = (uint8_t)(0.439f * r_value - 0.368f * g_value - 0.071 * b_value + 128);

            yuv_frame_data[pixel_i]                 = y_value;
            yuv_frame_data[pixel_i + PIXEL_NUM]     = u_value;
            yuv_frame_data[pixel_i + PIXEL_NUM * 2] = v_value;
        }
        byte_count = fwrite(
            &yuv_frame_data[0],
            sizeof(frame_data_1[0]), PIXEL_NUM * 3, 
            yuv_file
        );
        if (byte_count != PIXEL_NUM * 3)
        {
            fprintf(stderr, "[f%d - 2] Only wrote %lu bytes of original!\n", frame_i, byte_count);
            break;
        }

        /*************************************/
        /*** Undersampling 4:4:4 --> 4:2:0 ***/
        /*************************************/
        uint8_t *yuv_undersampled_data = &frame_data_2[0]; // Uses other buffer
        for (uint row = 0; row != PIXEL_HEIGHT / 2; ++row)
        {
            for (uint col = 0; col != PIXEL_WIDTH / 2; ++col)
            {
                // Average 4 U values into 1 (+ PIXEL_NUM because Y is preserved)
                yuv_undersampled_data[row * (PIXEL_WIDTH / 2) + col + PIXEL_NUM] = (
                    yuv_frame_data[  2 * row     * PIXEL_WIDTH + 2 * col     + PIXEL_NUM] +
                    yuv_frame_data[  2 * row     * PIXEL_WIDTH + 2 * col + 1 + PIXEL_NUM] +
                    yuv_frame_data[(2 * row + 1) * PIXEL_WIDTH + 2 * col     + PIXEL_NUM] +
                    yuv_frame_data[(2 * row + 1) * PIXEL_WIDTH + 2 * col + 1 + PIXEL_NUM]
                ) / 4;
                // Average 4 V values into 1 (+ PIXEL_NUM + PIXEL_NUM / 4 for Y and U offset)
                yuv_undersampled_data[row * (PIXEL_WIDTH / 2) + col + (PIXEL_NUM + PIXEL_NUM / 4)] = (
                    yuv_frame_data[  2 * row     * PIXEL_WIDTH + 2 * col     + (PIXEL_NUM + PIXEL_NUM / 4)] +
                    yuv_frame_data[  2 * row     * PIXEL_WIDTH + 2 * col + 1 + (PIXEL_NUM + PIXEL_NUM / 4)] +
                    yuv_frame_data[(2 * row + 1) * PIXEL_WIDTH + 2 * col     + (PIXEL_NUM + PIXEL_NUM / 4)] +
                    yuv_frame_data[(2 * row + 1) * PIXEL_WIDTH + 2 * col + 1 + (PIXEL_NUM + PIXEL_NUM / 4)]
                ) / 4;
            }
        }
        byte_count = fwrite(
            &yuv_undersampled_data[0],
            sizeof(frame_data_1[0]), PIXEL_NUM + PIXEL_NUM / 2, 
            yuv_undersampled_file
        );
        if (byte_count != PIXEL_NUM + PIXEL_NUM / 2)
        {
            fprintf(stderr, "[f%d - 3] Only wrote %lu bytes of original!\n", frame_i, byte_count);
            break;
        }

        /*************************************/
        /*** Oversampling  4:2:0 --> 4:4:4 ***/
        /*************************************/
        uint8_t *yuv_oversampled_data = &frame_data_1[0]; // Uses other buffer
        for (uint row = 0; row != PIXEL_HEIGHT / 2; ++row)
        {
            for (uint col = 0; col != PIXEL_WIDTH / 2; ++col)
            {
                yuv_oversampled_data[  2 * row     * PIXEL_WIDTH + 2 * col     + PIXEL_NUM] =
                    yuv_undersampled_data[row * (PIXEL_WIDTH / 2) + col + PIXEL_NUM];
                yuv_oversampled_data[  2 * row     * PIXEL_WIDTH + 2 * col + 1 + PIXEL_NUM] =
                    yuv_undersampled_data[row * (PIXEL_WIDTH / 2) + col + PIXEL_NUM];
                yuv_oversampled_data[(2 * row + 1) * PIXEL_WIDTH + 2 * col     + PIXEL_NUM] =
                    yuv_undersampled_data[row * (PIXEL_WIDTH / 2) + col + PIXEL_NUM];
                yuv_oversampled_data[(2 * row + 1) * PIXEL_WIDTH + 2 * col + 1 + PIXEL_NUM] =
                    yuv_undersampled_data[row * (PIXEL_WIDTH / 2) + col + PIXEL_NUM];
            }
        }
        byte_count = fwrite(
            &yuv_oversampled_data[0],
            sizeof(frame_data_1[0]), PIXEL_NUM * 3, 
            yuv_oversampled_file
        );
        if (byte_count != PIXEL_NUM * 3)
        {
            fprintf(stderr, "[f%d - 4] Only wrote %lu bytes of original!\n", frame_i, byte_count);
            break;
        }
    }
    
    /*************************/
    /*** CLOSE VIDEO FILES ***/
    /*************************/
    fclose(rgb_video_file);
    fclose(yuv_file);
    fclose(yuv_undersampled_file);
    fclose(yuv_oversampled_file);

    return 0;
}
