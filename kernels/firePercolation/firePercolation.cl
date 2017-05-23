//Use Conways.cpp for set up

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP_TO_EDGE|CLK_FILTER_NEAREST;

#define is_green(color) (color.x == 0.0 && color.y == 1.0)
#define is_fire(color) (color.x > 0.0 && color.y == 0.0)

__kernel void test_area(__read_only image2d_t in_image, __write_only image2d_t out_image){
	unsigned int x = get_global_id(0);
	unsigned int y = get_global_id(1);

	float4 green = (float4)(0.0, 1.0, 0.0, 0.0);
	float4 red   = (float4)(1.0, 0.0, 0.0, 0.0);

	float4 myColorValue;
	float4 valuesAround;

	myColorValue  = read_imagef(in_image, sampler, (int2)(x    , y));
	valuesAround  = read_imagef(in_image, sampler, (int2)(x    , y - 1));
	valuesAround += read_imagef(in_image, sampler, (int2)(x    , y + 1));
	valuesAround += read_imagef(in_image, sampler, (int2)(x - 1, y));
	valuesAround += read_imagef(in_image, sampler, (int2)(x + 1, y));

	if(is_fire(myColorValue)){
		color.x -= 1/255; 
	}
	else{
		if(is_green(color)){
			if(valuesAround.x > 0){
				myColorValue = red;
			}
		}
	}
	write_imagef(out_image, (int2)(x,y), myColorValue);
}