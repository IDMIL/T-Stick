# How to order touch sensor from JLCPCB and PCBWAY

This file contains instructions for ordering the touch-sensor from JLCPCB or PCBWAY. Both companies offer relatively cheap Flexible PCB options however JLCPCB is significantly cheaper for small runs.

## Ordering from JLCPCB
1. Click the blue **Instant Quote** button.
2. Select **Flex** as the Base material.
3. Click the blue **Add gerber file** button.
4. Select the **touch-sensor-horizontal.kicad_pcb.zip** from this folder and upload it. Several options should now be automatically filled out such as the layers and dimensions.
5. Select the appropriate PCB Qty.
6. Under **PCB Specifications** change _PCB Thickness_ to 0.2.
7. Under **High-spec Options** change the following settings:
    - _Gold Fingers_ = Yes
    - _Gold Fingers Thickness_ = 0.3mm
    - _Stiffener_ = Polymide + 3M Tape
    - _Polymide Thickness_ = 0.1mm
    - _3M Tape Thickness_ = 3M468
8. Click the blue _Save to Cart_ option and follow the prompts to pay for the PCB. You may choose to wait until review to pay but note that production will be delayed until payment. 
9. Make sure to check your inbox 24-72 hours after the purchase as JLCPCB might contact you regarding any questions about the order.

## Ordering from PCBWAY
1. Click the **PCB Instant Quote** button in the taskbar at the top of the screen.
2. Select **FPC/Rigid-Flex** PCB.
3. Change the following settings:
    - _Layers_ = 2
    - _Quantity_ = set as desired (note that small orders are really expensive)
    - _FPC Thickness_ = 0.2
    - _Edge Connector_ = Yes
    - _TOP PI_ = 0.1mm
    - _3M/Tesa Tape_ = 3M467, onesided BOT
    - _E-test_ = No (optional, to reduce cost)
4. Click save to cart.
5. In the next screen click the green **Add Gerber File** button. 
6. Select the **touch-sensor-horizontal.kicad_pcb.zip** from this folder and upload it. 
7. Make sure to check your inbox 24-72 hours after the purchase for PCBWAY to review the file and give the final quote.
8. Once the paid the order will go into production.
