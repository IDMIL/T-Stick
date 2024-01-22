# How to order board from PCBWAY

This file contains instructions for ordering the a PCB from PCBWAY for both fabrication and assembly. If you have the PCBWAY plugin from Kicad you can press use the plugin and skip straight to step 10

## Ordering from PCBWAY
1. Click the **PCB Instant Quote** button in the taskbar at the top of the screen.
2. Click the green **Quick-order PCB>>** button.
3. Press the grey **+Add Gerber File** button.
4. Select the **.kicad_pcb.zip** file. Several paramaeters should now autofill.
    - For a 4 layer board it may ask you to specify the layer names and order you should fill in the following:
        - L1 = F.Cu
        - L2 = In1.Cu
        - L3 = In2.Cu
        - L4 = B.Cu
5. Select the appropiate quantity.
6. Scroll further down to the **Assembly Service** section and click it.
7. Write the appropriate quantity of boards you wish to be assembled.
8. Click the green **Save to Cart** button.
9. You will now see two orders one for the fabrication and the other for assembly. Click the blue **Add files button for assembly**.
10. Click the **Parts List (BOM) Upload** and add the **BOM.csv** file for the board. 
    - Note that if you used the Kicad plugin you will need to remove the BOM file that has been uploaded as it does not have all the information needed.
11. _(Optional)_ Click the **Upload Centroid File** button and add the **PCBWay_positions.csv** file. This file is already in the **.kicad_pcb.zip** file but the customer support may require you to submit it seperately.
12. Make sure to check your inbox 24-72 hours after the purchase for PCBWAY to review the file and give the final quote.
13. Once the paid the order will go into production.