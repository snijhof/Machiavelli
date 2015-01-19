#include "Condottiere.h"
#include "CardColor.h"
#include "NetworkServices.h"

Condottiere::Condottiere() : PlayerCard("Condottiere")
{
}


Condottiere::~Condottiere()
{
}

std::string Condottiere::GetCharacteristicDescription()
{
	std::string output;
	
	output.append("As a Condottiere you are allowed to destroy 1 building from a other player. \n");
	output.append("Buildings worth 1 gold you can destroy for free, other buildings you pay 1 gold less then the price. \n");
	output.append("You will also earn 1 gold for every red building you have build.");
	
	return output;
}

void Condottiere::PerformCharacteristic(std::shared_ptr<GameManager> manager, std::shared_ptr<Player> player)
{
	/*
	Hij mag een gebouw bij een andere speler weghalen. Gebouwen die maar 1 goudstuk waard zijn mag hij
	kosteloos verwijderen. Voor andere gebouwen moet hij 1 goudstuk minder betalen dan ze hun eigenaar gekost
	hebben. Het goud wordt aan de bank betaald. Het verwijderde gebouw gaat op de aflegstapel. De condotierre
	mag ook gebouwen van vermoorde karakters verwijderen. Hij mag echter geen gebouwen verwijderen uit
	steden die al uit 8 of meer gebouwen bestaan. De condotierre ontvangt 1 goudstuk voor elk rood gebouw dat
	hij voor zich heeft liggen.
	*/

	// Destroy building from player

	// Receive gold for every build red building
	std::shared_ptr<Socket> socket = player->GetSocket();
	std::vector<std::shared_ptr<BuildCard>> builded_cards_list = player->GetBuildedBuildings();

	int counter = 0;
	for (int i = 0; i < builded_cards_list.size(); i++)
	{
		std::shared_ptr<BuildCard> build_card = builded_cards_list.at(i);
		if (build_card->GetColor() == CardColor::RED)
		{
			player->AddGold(1);
			counter++;
		}
	}

	std::shared_ptr<NetworkServices> networkServices = manager->GetNetworkServices();
	networkServices->WriteToClient("You received " + std::to_string(counter) + " gold.", socket, true);
}

PlayerCardType Condottiere::GetType()
{
	return PlayerCardType::CONDOTTIERE;
}
