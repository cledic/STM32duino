/****************************************************************************************************************************
  WebClient_SSL.ino - Dead simple SSL WebClient for Ethernet shields

  For STM32F/L/H/G/WB/MP1 with built-in Ethernet LAN8742A (Nucleo-144, DISCOVERY, etc) or W5x00/ENC28J60 shield/module
  
  EthernetWebServer_SSL_STM32 is a library for STM32 using the Ethernet shields to run WebServer and Client with/without SSL

  Use SSLClient Library code from https://github.com/OPEnSLab-OSU/SSLClient
  
  Built by Khoi Hoang https://github.com/khoih-prog/EthernetWebServer_SSL_STM32
 *****************************************************************************************************************************/
#include <STM32SD.h>
#define SD_DETECT_PIN PC13

#include "LTDC_F746_Discovery.h"
#include <PNGdec.h>

/* **** SDRAM Include and #DEFINE ************************************* */
#include "stm32f7xx_hal_sdram.h"

SDRAM_HandleTypeDef hsdram1;
#define REFRESH_COUNT                            ((uint32_t)0x0603)   /* SDRAM refresh counter (100Mhz SD clock) */
#define SDRAM_TIMEOUT                            ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000) 
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200) 
/* ******************************************************************** */

int16_t w, h, xOffset=0, yOffset=0;

LTDC_F746_Discovery tft;
// 480x272 pixel
#define H_RES 480
#define V_RES   272
__attribute__ ((section(".extram"))) uint16_t buffer2[H_RES * V_RES];

File pngFile;

#include "defines.h"

// You must have SSL Certificates here
// https://openslab-osu.github.io/bearssl-certificate-utility/
#include "trust_anchors_ilmeteo.h"

const char      server_host[]   = "www.ilmeteo.it"; // leave this alone, change only above two
const uint16_t  server_port     = 443;
String readHttp;

PNG png;
uint16_t usPixels[480];
uint8_t usMask[480];

// Initialize the SSL client library
// Arguments: EthernetClient, our trust anchors
EthernetClient    EthClient;
EthernetSSLClient sslClient(EthClient, TAs, (size_t)TAs_NUM);

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;

bool printWebData = true;  // set to false for better speed measurement

void *myOpen(const char *filename, int32_t *size)
{
  pngFile = SD.open(filename, FILE_READ);


  if (!pngFile || pngFile.isDirectory())
  {
    Serial.print(F("ERROR: Failed to open "));
    Serial.print(filename);
    Serial.println(F(" file for reading"));
  }
  else
  {
    *size = pngFile.size();
    Serial.printf("Opened '%s', size: %d\n", filename, *size);
  }

  return &pngFile;
}

void myClose(void *handle)
{
  if (pngFile)
    pngFile.close();
}

int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!pngFile)
    return 0;
  Serial.printf("myRead: %d\r\n", length);
  return pngFile.read(buffer, length);
}

int32_t mySeek(PNGFILE *handle, int32_t position)
{
  if (!pngFile)
    return 0;
  Serial.printf("mySeek: %d\r\n", position);
  return pngFile.seek(position);
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw)
{
  
  // Serial.printf("Draw pos = 0,%d. size = %d x 1\n", pDraw->y, pDraw->iWidth);
  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0x00000000);
  png.getAlphaMask(pDraw, usMask, 1);
  //tft.drawRGBBitmap(xOffset, yOffset + pDraw->y, usPixels, usMask, pDraw->iWidth, 1);
  tft.drawRGBBitmap(0, pDraw->y, usPixels, usMask, pDraw->iWidth, 1);

  Serial.printf("PNGDraw: iWidth: %d\r\n", pDraw->iWidth);
}

void setup()
{
  /* Configure external SDRAM */
  SDRAM();
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  Serial.println("Init Display!");Serial.flush();

  tft.begin((uint16_t *)buffer2);
  tft.fillScreen( LTDC_BLACK );
  tft.setRotation(1); // horizontal wide screen
  tft.setCursor(0, 2);
  tft.setTextColor(LTDC_GREEN);  //tft.setTextSize(3);
  tft.setTextSize(1);

  Serial.print("\nStart WebClient_SSL on " + String(BOARD_NAME));
  Serial.println(" with " + String(SHIELD_TYPE));

#if USE_ETHERNET_GENERIC
  Serial.println(ETHERNET_GENERIC_VERSION);
#endif
  
  Serial.println(ETHERNET_WEBSERVER_SSL_STM32_VERSION);

  Serial.print("Connecting... ");
  // start the ethernet connection and the server:
  // Use DHCP dynamic IP and random mac
  // uint16_t index = millis() % NUMBER_OF_MAC;
#if 0
  // Use Static IP
  Ethernet.begin(mac[0], ip, dns, gw);
#else  
  uint32_t net_res = Ethernet.begin(mac[0]);
  if ( !net_res)
  {
    Serial.println("ERROR: No IP\r\n");
    while(1);
  }
#endif

  Serial.print("Connected! IP address: ");
  Serial.println(Ethernet.localIP());

  // give the Ethernet shield a second to initialize:
  delay(2000);

  Serial.print("Connecting to : ");
  Serial.print(server_host);
  Serial.print(", port : ");
  Serial.println(server_port);

  // if you get a connection, report back via serial:
  auto start = millis();

  // specify the server and port, 443 is the standard port for HTTPS
  if (sslClient.connect(server_host, server_port))
  {
    auto time = millis() - start;
    Serial.print("Connected to ");

#if ( USE_ETHERNET || USE_ETHERNET_LARGE || USE_ETHERNET_ENC || USE_UIP_ETHERNET )
    Serial.println(EthClient.remoteIP());  
#else    
    Serial.println(server_host); 
#endif
    
    Serial.print("Took: ");
    Serial.println(time);

    // Make a HTTP request:
    //sslClient.println("GET /asciilogo.txt HTTP/1.1");
    sslClient.println("GET /cartine3/0.LAZ.png HTTP/1.1");
    sslClient.println("User-Agent: SSLClientOverEthernet");
    sslClient.print("Host: ");
    sslClient.println(server_host);
    sslClient.println("Connection: close");
    sslClient.println();
  }
  else
  {
    // if you didn't get a connection to the server:
    Serial.println("Connection failed!");
    while(1);
  }

  beginMicros = micros();

  Serial.print(F("Initializing SD card..."));

  while (!SD.begin(SD_DETECT_PIN))
  {
    delay(10);
  }
  Serial.println("SD Initialization done!");
  
}

void loop()
{
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (sslClient.available()) 
  {
    char c = sslClient.read();
    readHttp += c; 
    byteCount++;
  }

  // if the server's disconnected, stop the sslClient:
  if (!sslClient.connected())
  {
    
    endMicros = micros();

    Serial.println();
    Serial.println("Disconnecting.");
    sslClient.stop();

    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(" s, rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();


    int idx = readHttp.indexOf("\r\n\r\n")+4;
    int ln = readHttp.length()-idx;

    Serial.printf("idx: %d, ln: %d\r\n", idx, ln);
    
    /*
     * I send the HEX stream received to the console and use this site to save it as a file:
     * https://tomeko.net/online_tools/hex_to_file.php?lang=en
     * 
     * The file saved is a good .PNG file, as expected.
     */
    const char *img = readHttp.c_str();
         
    Serial.printf("\n");
    for( int i=0; i<ln; i++)
    {
      Serial.printf("%02X ", img[i+idx]);
    }
    Serial.printf("\n");

    /* I saved the .PNG image on SDCard to try using the "png.open" command */
    Serial.println("Creating image.png file...");
    pngFile = SD.open("image.png", FILE_WRITE);
    uint32_t byteswritten = pngFile.write(img, ln);
    pngFile.close();
    if ( byteswritten != ln)
    {
      Serial.println("ERROR: myFile.write!\r\n");
      while(1);
    } else {
      Serial.printf("Image Stored! [%d Byte]\r\n", byteswritten);
    }

    /*
     * If I enable the code below, the Ethernet client will stop to work.
     * The board is no mode capable to take an IP using the DHCP even to use a static IP
     * I tried both method: "png.open" and "png.openRAM" but with the same result.
     */
#if 0
    int rc = png.open("image.png", myOpen, myClose, myRead, mySeek, PNGDraw);
    if (rc == PNG_SUCCESS)
    {          
      rc = png.decode(NULL, 0);      
      png.close();
    }
#endif    
    
    // do nothing forevermore:
    while (true)
    {
      delay(1);
    }
  }
}

/* **** Code to activate externa SDRAM ******************************* */
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram) 
{
  GPIO_InitTypeDef gpio_init_structure;
  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  /* Common GPIO configuration */
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_PULLUP;
  gpio_init_structure.Speed = GPIO_SPEED_FAST;
  gpio_init_structure.Alternate = GPIO_AF12_FMC;
  /* GPIOC configuration */
  gpio_init_structure.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOC, &gpio_init_structure);
  /* GPIOD configuration */
  gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9
      | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);
  /* GPIOE configuration */
  gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8
      | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
      | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);
  /* GPIOF configuration */
  gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
      | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
      | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &gpio_init_structure);
  /* GPIOG configuration */
  gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5
      | GPIO_PIN_8 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);
  /* GPIOH configuration */
  gpio_init_structure.Pin = GPIO_PIN_3 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);
}

void SDRAM() 
{
  FMC_SDRAM_TimingTypeDef SdramTiming;
  hsdram1.State = HAL_SDRAM_STATE_RESET;
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 7;
  SdramTiming.WriteRecoveryTime = 3;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  HAL_SDRAM_MspInit(&hsdram1);
  HAL_SDRAM_Init(&hsdram1, &SdramTiming);
  BSP_SDRAM_Initialization_sequence(REFRESH_COUNT);
}

/**
  * @brief  Programs the SDRAM device.
  * @param  RefreshCount: SDRAM refresh counter value 
  * @retval None
  */
void BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount)
{
  __IO uint32_t tmpmrd = 0;
  FMC_SDRAM_CommandTypeDef Command;
  
  /* Step 1: Configure a clock configuration enable command */
  Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

  /* Step 2: Insert 100 us minimum delay */ 
  /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
  HAL_Delay(1);
    
  /* Step 3: Configure a PALL (precharge all) command */ 
  Command.CommandMode            = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);  
  
  /* Step 4: Configure an Auto Refresh command */ 
  Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 8;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);
  
  /* Step 5: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |\
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
                     SDRAM_MODEREG_CAS_LATENCY_2           |\
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  
  Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);
  
  /* Step 6: Set the refresh rate counter */
  /* Set the device refresh rate */
  HAL_SDRAM_ProgramRefreshRate(&hsdram1, RefreshCount); 
}
/* ******************************************************************* */
