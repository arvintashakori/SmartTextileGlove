struct bno055_t myBNO;

// Link the I2C driver functions to the API communication function pointer
// to the API communication function pointer
myBNO.bus_read=BNO055_I2C_bus_read;
myBNO.bus_write = BNO055_I2C_bus_write; 
myBNO.delay_msec = delay;

// Set the correct I2C address in the BNO055 API
myBNO.dev_addr = BNO055_I2C_ADDR1;
//myBNO.dev_addr = BNO055_I2C_ADDR2;

bno055_init(&myBNO); // API initialization

//Change the operation to NDOF
bno055_set_operation_mode(OPERATION_MODE_NDOF);

struct bno055_euler_float_t eulerData; // Read Euler angles
bno055_convert_float_euler_hpr_deg(&eulerData);

unsigned char accel_calib_status = 0; // Read calibration status
unsigned char gyro_calib_status = 0; 
unsigned char mag_calib_status = 0; 
unsigned char sys_calib_status = 0;
bno055_get_accel_calib_stat(&accel_calib_st atus);
bno055_get_mag_calib_stat(&mag_ calib_status); 
bno055_get_gyro_calib_stat(&gyro_ calib_status); 
bno055_get_sys_calib_stat(&sys_ calib_status);