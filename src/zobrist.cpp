#include <cassert>
#include <cstdlib>
#include <unordered_map>
#include "zobrist.hpp"

const size_t ZOBRIST_TABLE_SIZE = 64 * 0x100000;

ZobristValue zobrist_table[ZOBRIST_TABLE_SIZE];

// 7 tiles wide, 7 tiles tall, 5 states per tile (SQUARE_EMPTY, PLAYER1, PLAYER2, PLAYER3, PLAYER4)
const ZobristHash ZOBRIST_CODES[7*7*5] = {
    0xD3B4A1DCF04B95E7LL, 0xC2BEA6C6F6A32C88LL, 0x3FA3734AB3E58F12LL, 0x683C4713574928EFLL, 0x9F6339D757DF669FLL,
    0x1B7BC1284D66C569LL, 0x400F8D31BD799682LL, 0x2DF51903CD46755ALL, 0xF13480B0426D9F90LL, 0x8306BECC9A68229CLL,
    0xA5BCB8A4C41275C1LL, 0xBA74CF92C276FBC4LL, 0xE0FD66026C3FEB74LL, 0x274E6DAB0C7FFCA7LL, 0x026221774A11C18ALL,
    0x743FA84729BF1527LL, 0x8FF1DA556499B73FLL, 0xB930415A1B3717A7LL, 0x3E7D5B226527475BLL, 0xCF8772A52E004F9ELL,
    0x6EDCEA00D9290A87LL, 0x5BAB57481782CFEALL, 0xAF0D756B88E7DC98LL, 0xCF188195AD3E407FLL, 0x69BA0F013B310A10LL,
    0x47201AC2DB11A5BELL, 0x6D5A521EDC425434LL, 0x8635DBB93C3E629BLL, 0xCE36D0C8A7A0BAA0LL, 0x2CDB95FBCDFA5174LL,
    0x256A4C6CA1B76F6FLL, 0x2AC998A85CC5E4C5LL, 0xC8A08D3A0952E120LL, 0xCDC0CBCB24376434LL, 0x73C1868F9930E54BLL,
    0xEE507962AA263C04LL, 0xB1FA7C7C9B28C970LL, 0x71F69C1D7D662254LL, 0x5F500D3941254395LL, 0xAF17B9F446AADC52LL,
    0x52FB8F403BA18FA1LL, 0xFC6931D8856A2B9DLL, 0x3643BD34C2FC8B8BLL, 0x9C5E6B9000A5EB89LL, 0x4C8271DE72029541LL,
    0x263E1A702D7AE531LL, 0xEA11D6455EAE76E6LL, 0x7F632E5D553652F5LL, 0x0E618ED7BE95369FLL, 0xCF7273C88E911A20LL,
    0x391CA6AFB231C1A4LL, 0xB4D7040872817B9ELL, 0x80B06117E7AEA97ALL, 0xC2CD5E1D1572428DLL, 0xDA80EE01BA7E9DB4LL,
    0x8E626F8791C9CC64LL, 0xD2662FA5F253170CLL, 0x35907B0AB15B0D6ELL, 0xBC544FCA981D0C3FLL, 0x7A7D3EE5B3492EA4LL,
    0x588EC7A758D29569LL, 0xE3D06DA147E907A2LL, 0x88D0EA1508675E1FLL, 0x22955F6EFD145A9ELL, 0x6D15037FD0642753LL,
    0xB098D0F356A9B687LL, 0xB921060D26238852LL, 0x9A88A19DF60CDBC1LL, 0x8998DA0D46962CC7LL, 0x068FE0638A531CFCLL,
    0xB7F959B88042A3F5LL, 0xCB9E8733760CE9FCLL, 0x27F59223A048DB10LL, 0x56169C9C6734CAC0LL, 0x2FAD29231C3A8226LL,
    0xFF5AD1844CBE6F46LL, 0x80F4DCC08F49C6E7LL, 0x36E97F7B03904C72LL, 0x1883AB7B7353AEE1LL, 0x07F4E64C97FEEC8ELL,
    0xE95EADDE3804D4DBLL, 0xA10B2DAB188F4F19LL, 0xA23EE03D87A759A2LL, 0x902B846CB4670ACDLL, 0xEF8AE479B21F5A18LL,
    0x80EF92DF8E6D2B02LL, 0x664D5990EAE11C7FLL, 0x191E42AF87F67140LL, 0x62A42961D100FB74LL, 0x11F5FFE0F5CE9BE6LL,
    0xDF76ECD164AFB746LL, 0xA92105E0083DB610LL, 0x18699934758DAD39LL, 0xA0D18AE63BD266DELL, 0xD080CCB8EB04B95FLL,
    0x3CA674DA550DD1EBLL, 0x4251E5775548DCC9LL, 0x05C3DC537E399D42LL, 0x66249D0DAC74034FLL, 0xFE8F9C70BA737752LL,
    0x7E0CA70F1AD4557ELL, 0x4B8639D800A72B5DLL, 0xA879077191428CF2LL, 0xB4C9FF08A22710D6LL, 0x97DCCA6D9F38C91ALL,
    0x203AACA483845737LL, 0x6E8F6F397F78DD42LL, 0xF5EA17ED536F9652LL, 0x87487F4EDDF8E970LL, 0xFDE82368D9FB0591LL,
    0x2966B0ABBD06B0D4LL, 0x3E11BAF75A1A15D0LL, 0x938390191B07A47BLL, 0x91308119D5C65501LL, 0x6CD05236FA49C2B2LL,
    0x1F03488257A6C4B7LL, 0x8AE5FABB33A93291LL, 0xC71229FA51A7F1D2LL, 0xA24F50DA834A71B4LL, 0x1811C95DB0E54B02LL,
    0x7343B99EEB9C7A8BLL, 0x0644E9A205D68DEDLL, 0x6DA76E57BAAC4F10LL, 0xE332D4734E89F7C1LL, 0x9CCE346B2780767FLL,
    0x24A86AEAB9BC24A1LL, 0xA5223640F14C432ALL, 0x6DC0DEBD0C62E484LL, 0x727BBF317B116730LL, 0xBA9605F3553466B3LL,
    0x56E2735B0DF2B27ALL, 0x599510176C047485LL, 0x0C87C1FE2191BAB8LL, 0x7E31E64481E4CCA4LL, 0xEB31BD02A5A6772CLL,
    0x045C5B95E99412F7LL, 0x56DAB4366D14F25DLL, 0xA2946D09184CC5AFLL, 0x4134FD02170F8B43LL, 0xF1C687999C703D5DLL,
    0x1A849ED67204E34ALL, 0x01B2C9BE703575BCLL, 0xB0BD1B243C2F508ALL, 0xB495DC4A4064C7DELL, 0x4FC1CD0B00574984LL,
    0x22EF28C7705C40ACLL, 0xBB7F7AB0DDFC2337LL, 0xACFC9D64F2DC310BLL, 0x1BA5B8572544D92ELL, 0xDBD2E5E83E4CEEC0LL,
    0xC7F62413F85BDD47LL, 0xB6B69BA402882CCALL, 0xC84A1E3B6F5B0D62LL, 0xFFAD01B700C350EFLL, 0x32701853E09D4BCFLL,
    0x0E400CF16A79A703LL, 0x67F406F30F9D471ALL, 0x907ADC8CE37AA13ALL, 0x0F75F9801F236978LL, 0x94ABBFDC18082BEFLL,
    0x3330C08AC0803D36LL, 0x3F704927BEB8595ELL, 0x4EFB67FFD4F2ED65LL, 0xA9BC1F6CD2FCE8EALL, 0x5FED69FB9487AD8ELL,
    0x75B1AE6292888AC2LL, 0x38F7A9AF8B140F30LL, 0xCA43DCB35912A867LL, 0x01380FFE549C672BLL, 0x0BDFE8A816926362LL,
    0x3040273A5F928F01LL, 0xD2916D2DDE616CDDLL, 0x6196AFDECC0C48ADLL, 0x54CDDB8DF2076013LL, 0x2D24C53ADFD286DCLL,
    0x51AA4063064C9C66LL, 0xA376A933E2AAC360LL, 0xF1D8BDBF91E86238LL, 0x3A0CC0AC8A46057ELL, 0xEABB27F17569CF3DLL,
    0x01A86BE787A61AAFLL, 0x29EDF2DA84554A3ELL, 0x5BBFC1CEC7C1401DLL, 0x09899862CA449232LL, 0x36F1270A3FC24D18LL,
    0x7E9214F47042DBA4LL, 0x3E14B58E5AA3F28ELL, 0xBCEDE11ACC23FA77LL, 0x06B6BB35B59A9034LL, 0x97B7BA1C76E3BA50LL,
    0x921AF75B72DA6AB6LL, 0x75CBABDC8EA40BAFLL, 0x83C16AC85B428041LL, 0x7C51DA5BA519B199LL, 0x7D6938B14C952DC5LL,
    0x15FECD12EEF04316LL, 0x4A388928A56FDFC6LL, 0xE85E4EB9E8744C98LL, 0x3743C15A06B4C77FLL, 0x8138363502A88D4BLL,
    0x4F4763641D9E0AD7LL, 0x03915BCCCFBE38ACLL, 0x2A9C6CA76A134F4FLL, 0xBF9679369BB397ABLL, 0x4106EFFBA9368D22LL,
    0xA12A65EC551C24BALL, 0x7C4C32F6BB30CE0CLL, 0x910B126212508528LL, 0x8EB7EECE5ABBC7C4LL, 0xEFAA183F418E3ACCLL,
    0x4FDA0D82096C5AF5LL, 0xE2CAFD1067838166LL, 0xEB628568FAFE472FLL, 0x83A8DF492CF2C1CDLL, 0x44937CC268FCB995LL,
    0x005CCE9157BD197DLL, 0x1B1B4062DDC40982LL, 0x12AD2941191C69B0LL, 0x9D731CF232D0BF4CLL, 0x92162A82D919A537LL,
    0xE4A962843590FC87LL, 0x3B047876A6D47D85LL, 0xEA7FBEC8DB7D53E6LL, 0x1EF95F8F6C75A948LL, 0xA8D8DC0D8C62EB6DLL,
    0x436C5B8B938035C5LL, 0x52126126D4C22739LL, 0x2A4A38C112EE0A00LL, 0x9029ED5876183B5CLL, 0x4BB6D8C95DD198B2LL,
    0x56C64949672F81B4LL, 0x919F9D3B94FB0B07LL, 0x374223EC87905858LL, 0x4783DE15420E3928LL, 0x17AF493A4A00A14ELL,
    0x6F6D220F9EFA7329LL, 0x622CC844835AC75FLL, 0x433C8EF06EA87458LL, 0x38EDA0D3FF6044A6LL, 0xB08A69B2F4749089LL,
    0xD0EB64A7F7BE3B3FLL, 0x67276B47AA5CAB5ALL, 0xC509F296E603FC01LL, 0xC8C7C92F1AD8DEDALL, 0x942A4B02AA97A5D5LL
};

// XORed into hash when it's player 2's turn (@TODO@ -- codes for player 3, player 4)
const ZobristHash PLAYER2_TURN_CODE = 0x431D89EC63B226D7LL;


ZobristHash calcHash(const Board& board, int turn);


void initZobristTable()
{
    // Mark everything in the table as invalid
    for(ZobristValue& value : zobrist_table) {
        value.depth = -1;
    }
}

// turn is 1 for AI, -1 for human
void getZobristValue(const Board& board, int turn, ZobristValue &value)
{
    ZobristHash hash = calcHash(board, turn);
    value = zobrist_table[hash % ZOBRIST_TABLE_SIZE];
    if(value.full_hash != hash) {
        // The short hash (i.e. mod ZOBRIST_TABLE_SIZE) collided, but the full hash did not.
        // That means this result is spurious; invalidate it.
        value.depth = -1;
    }
}

void setZobristValue(const Board& board, int turn, ZobristValue &value)
{
    ZobristHash hash = calcHash(board, turn);
    value.full_hash = hash;
    zobrist_table[hash % ZOBRIST_TABLE_SIZE] = value;
}

ZobristHash calcHash(const Board& board, int turn)
{
    ZobristHash hash = 0;
    const Player* board_cell = &board(0, 0);
    size_t count = 0;
    for(int y = 0; y < 7; ++y) {
        for(int x = 0; x < 7; ++x) {
            hash ^= ZOBRIST_CODES[*board_cell++ + count++];
        }
    }
    if(turn == 1) {
        hash ^= PLAYER2_TURN_CODE;
    }
    return hash;
}
