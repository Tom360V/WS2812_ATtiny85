/******************************************************************************
 * tiny85.c
 *
 * Created: 18-1-2017 15:09:05
 * Author : Tom Koene
 *
 *****************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>         //required for rand()

/******************************************************************************
 * DEFINES
 *****************************************************************************/
//#define GREEN     (0)
//#define RED       (1)
//#define BLUE      (2)

#define P0     (1<<PINB0)
#define P1     (1<<PINB1)
#define P2     (1<<PINB2)
#define P3     (1<<PINB3)
#define P4     (1<<PINB4)
#define P5     (1<<PINB5)

/******************************************************************************
 * DEFINES FOR CONFIGURATION
 *****************************************************************************/
#define NOF_LEDS    (8)         //Specify number of leds here!
#define PB_MASK     (P3|P4)     //Specify which pins are INPUT

/******************************************************************************
 * PRIVATE TYPES
 *****************************************************************************/
typedef struct
{
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} color_t;

typedef enum
{
    eBuildState_Unkown      = -1,
    eBuildState_Nothing     = 0,
    eBuildState_Building    = P4,
    eBuildState_Succeed     = P3,
    eBuildState_Failed      = P3|P4,
} eBuildState_t;

/******************************************************************************
 * PRIVATE VARIABLE
 *****************************************************************************/
 uint8_t my_array[NOF_LEDS * 3] = {0x00};
uint8_t my_array_sizeof = NOF_LEDS *3;

/******************************************************************************
 * PRIVATE PROTOTYPES
 *****************************************************************************/
void UpdateLeds(void);

/******************************************************************************
 * PRIVATE FUNCTIONS
 *****************************************************************************/

/*
 * Wait for undifened amount of time
 * just burn some cycles
 */
void wait()
{
    uint16_t wait_cnt = 10000;
    while( (--wait_cnt) > 0)
    {
        __asm("nop");
    }
}

/*
 * Increment 'c' with 'step' as long max is not reached
 */
uint8_t incColor(uint8_t *c, uint8_t step)
{
    if(*c < 255-step)
    {
        (*c)+=step;
        return 1;
    }
    *c = 255;
    return 0;
}

/*
 * decrement 'c' with 'step' as long min is not reached
 */
uint8_t decColor(uint8_t *c, uint8_t step)
{
    if(*c > step)
    {
        *c-=step;
        return 1;
    }
    *c = 0;
    return 0;
}

/*
 * Shift rgb-array by 3 (from begin to end)
 * 3 because the array contains RGB values, like:
 *      G1 R1 B1 G2 R2 B2 G3 R3 B3 ... etc
 */
void shiftArray_2End(uint8_t *array, uint8_t length)
{
    array += (length-1);
    while(length>3)
    {
        *array = *(array-3);
        array--;
        length--;
    }
}

/*
 * Shift rgb-array by 3 (from end to begin)
 * 3 because the array contains RGB values, like:
 *      G1 R1 B1 G2 R2 B2 G3 R3 B3 ... etc
 */
void shiftArray_2Begin(uint8_t *array, uint8_t length)
{
    while(length>3)
    {
        *array = *(array+3);
        array++;
        length--;
    }
}

/*
 * Shift rgb-array by 3 (from begin to end)
 * And decrease all values in the array with 'dec'
 * 3 because the array contains RGB values, like:
 *      G1 R1 B1 G2 R2 B2 G3 R3 B3 ... etc
 */
void shiftArray_2EndAndDecrease(uint8_t *array, uint8_t length, uint8_t dec)
{
    array += (length-1);
    while(length>3)
    {
        *array = *(array-3);
        decColor(array, dec);
        array--;
        length--;
    }
}

/*
 * Color Pattern: Blue 'Nightrider'
 */
void buildingLight(uint8_t *arr)
{
    #define FACT    (6)
    uint8_t *led = arr+2;
    static int8_t idx = -FACT;
    static uint8_t dir = 1;
    static uint8_t intensity = 255;
    if(dir==0)
    {
        shiftArray_2End(my_array, my_array_sizeof);
        intensity /= FACT;
        *led = intensity;
        idx++;
        if(idx==NOF_LEDS-1)
        {
            intensity = 255;
            dir = 1;
        }
    }
    else
    {
        shiftArray_2Begin(my_array, my_array_sizeof);
        intensity /= FACT;
        *(led+((NOF_LEDS-1)*3)) = intensity;
        idx--;
        if(idx<-FACT)
        {
            intensity = 255;
            dir = 0;
        }
    }
}

/*
 * Color Pattern: Fancy RGB color loop
 */
void FancyColorLoop(uint8_t *r, uint8_t *g, uint8_t *b)
{
    static int fase=0;
    const uint8_t step = 20;

    wait();
    shiftArray_2Begin(my_array, my_array_sizeof);

    switch(fase)
    {
        case 0: 
            decColor(b, step);
            if(incColor(r,step) == 0)
            {
                fase++;
            }
            break;
        case 1:
            decColor(r, step);
            if(incColor(g,step) == 0)
            {
                fase++;
            }
            break;
        case 2:
            decColor(g, step);
            if(incColor(b,step) == 0)
            {
                fase = 0;
            }
            break;
    } // end switch
};

/*
 * Color Pattern: fire-ish-simulation, feel free to improve :)
 */
void fire(uint8_t *r, uint8_t *g, uint8_t *b)
{
    static uint16_t prevDemp = 0;
    uint8_t demping;

    if(rand()%4 == 2)
    {
        demping = rand()%128+10;
        prevDemp += demping;
        prevDemp /= 2;
        demping = prevDemp;
    }
    wait();
    shiftArray_2EndAndDecrease(my_array, my_array_sizeof, demping);
    
    *r = 255;
    *g = 255-(demping*2);
};

/*
 * Update all leds in the array to the same color
 */
void SetAllColor(uint8_t *arr, uint8_t length, color_t RGB)
{
    uint8_t idx = 0;
    while(idx<length)
    {
        *(arr++) = RGB.green;
        *(arr++) = RGB.red;
        *(arr++) = RGB.blue;
        idx+=3;
    }
}

/*
 * Check eBuildState and set correct light scene
 */
void SetBuildState(eBuildState_t eBuildState)
{
    static color_t c;
    static eBuildState_t ePrevBuildState = eBuildState_Unkown;
    if(ePrevBuildState != eBuildState)
    {
        ePrevBuildState = eBuildState;
        c.green = 0;
        c.blue = 0;
        c.red = 0;
        SetAllColor(my_array, my_array_sizeof, c);
    }

    switch(eBuildState)
    {
        case eBuildState_Nothing:
            c.green = 0;
            c.blue = 0;
            c.red = 0;
            SetAllColor(my_array, my_array_sizeof, c);
            my_array[0] = 10;
            my_array[1] = 10;
            my_array[2] = 10;
            UpdateLeds();
            break;
            
        case eBuildState_Building:
            buildingLight(my_array);
            UpdateLeds();
            break;
            
        case eBuildState_Succeed:
            c.green = 255;
            SetAllColor(my_array, my_array_sizeof, c);
            UpdateLeds();
            break;
        
        case eBuildState_Failed:
            if(c.red==0) {c.red=1;}
            c.red += c.red;
            SetAllColor(my_array, my_array_sizeof, c);
            UpdateLeds();
            break;

        default:
            c.red = 0;
            c.green = 0;
            c.blue = 0;
            SetAllColor(my_array, my_array_sizeof, c);
            UpdateLeds();
            break;
    }
}

/******************************************************************************
 * Main loop
 * Check pins from port B, if something is changed: update buildstatus
 *****************************************************************************/
int main(void)
{
    PLLCSR |= (1<<PLLE);
    CLKPR  |= (1<<CLKPS0);
    OSCCAL = 0xFF;
    
    DDRB = 0x01;    //0xFF = all output
    PORTB = 0x00;

    uint16_t timeout = 0;
    uint8_t portValue, prevPortValue;

    UpdateLeds();
    //sei();        //Enable Global Interrupt
    while (1) 
    {
#if 0
//        fire(&my_array[RED],&my_array[GREEN],&my_array[BLUE]);
        FancyColorLoop(&my_array[RED],&my_array[GREEN],&my_array[BLUE]);
      //buildingLight(my_array);
        UpdateLeds();

#else
        portValue = (PINB & PB_MASK);
        
        // Check if PINB has changed, or timeout is true
        if( (timeout == 0) ||
            (prevPortValue != portValue) )
        {
            prevPortValue = portValue;
            timeout = 50000;
            SetBuildState(portValue);
        }
        else
        {
            timeout--;
        }
#endif
    }
}