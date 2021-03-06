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
	std::string output = "";
	std::string name_destroyed_building = "";
	std::shared_ptr<Player> chosen_player;
	std::shared_ptr<Socket> socket = player->GetSocket();
	std::shared_ptr<NetworkServices> networkServices = manager->GetNetworkServices();

	// Print all the players
	bool players_available = false;
	networkServices->WriteToClient("Players: \n", socket, true);
	std::vector<std::shared_ptr<Player>> players = manager->GetPlayers();
	for (int i = 0; i < players.size(); i++)
	{
		std::vector<std::shared_ptr<BuildCard>> build_cards = players.at(i)->GetBuildedBuildings();
		if (players.at(i) != player && build_cards.size() > 0 && build_cards.size() < 8 && !players.at(i)->ContainsPlayerCard(PlayerCardType::PREACHER, false))
		{
			players_available = true;
			output.append(std::to_string(i) + ": " + players.at(i)->GetName() + "\n");
			output.append("Buildings: \n");
			for (int x = 0; x < build_cards.size(); x++)
			{
				output.append("- Name: " + build_cards.at(x)->GetName() + ", Cost: " + std::to_string(build_cards.at(x)->GetCost() - 1) + ", Color: " + build_cards.at(x)->GetColorString() + "\n");
			}
		}
	}

	if (players_available)
	{
		output.append("Stop: Will stop the characteristic.\n\n");
		output.append("Choose the number of the player you want to destroy a building from: \n");
		networkServices->WriteToClient(output, socket, true);

		// Choose a player
		output.clear();
		bool player_chosen = false;
		while (!player_chosen)
		{
			output.clear();
			std::string input = Utils::ToLowerCase(networkServices->PromptClient(player));

			if (input == "stop")
			{
				player->SetUsedCharacteristic(true);
				return;
			}

			try
			{
				int number = std::atoi(input.c_str());

				// Check if this number exists
				if (number < players.size())
				{
					chosen_player = players.at(number);
					output = "You chose player: " + chosen_player->GetName();
					player_chosen = true;
				}
				else
					output = "This is not a valid choice.\n";
			}
			catch (...)
			{
				output = "This is not a valid choice.\n";
			}

			networkServices->WriteToClient(output, socket, true);
		}
		output.clear();
		output.append("\n");
		networkServices->WriteToClient(output, socket, true);

		// Show all the buildings with the cost to destroy
		output.clear();
		output.append("Buildings: \n");
		std::vector<std::shared_ptr<BuildCard>> build_cards = chosen_player->GetBuildedBuildings();
		for (int i = 0; i < build_cards.size(); i++)
		{
			if (build_cards.at(i)->GetBuildingType() != BuildingEnum::KEEP)
				output.append(std::to_string(i) + ": " + build_cards.at(i)->GetName() + ", Cost: " + std::to_string(build_cards.at(i)->GetCost() - 1) + ", Color: " + build_cards.at(i)->GetColorString() + "\n");
		}
		output.append("Stop: Will stop the characteristic.\n\n");
		output.append("Choose the number of the building you want to destroy: \n");
		networkServices->WriteToClient(output, socket, true);

		// Destroy a building from the player
		output.clear();
		bool building_destroyed = false;
		while (!building_destroyed)
		{
			output.clear();
			std::string input = Utils::ToLowerAndTrim(networkServices->PromptClient(player));

			if (input == "stop")
			{
				player->SetUsedCharacteristic(true);
				return;
			}

            int number = Utils::ParseInt(input);
            if (number == -1)
                networkServices->WriteToClient("This is not a valid option.\n", socket);

            // Check if this number exists
            if (number < build_cards.size())
            {
                if (build_cards.at(number)->GetBuildingType() != BuildingEnum::KEEP)
                {
                    if (player->GetGold() >= (build_cards.at(number)->GetCost() - 1))
                    {
                        // Remove the building
                        std::shared_ptr<BuildCard> build_card = std::make_shared<BuildCard>(*build_cards.at(number).get());
                        chosen_player->DestroyBuilding(build_card);
                        manager->AddBuildCard(build_card);
                        building_destroyed = true;

                        name_destroyed_building = build_card->GetName();
                        player->RemoveGold(build_card->GetCost() - 1);

                        // Set the output
                        output.append("You have destroyed building: " + name_destroyed_building + ", ");
                        output.append("it cost you " + std::to_string(build_card->GetCost() - 1) + ", ");
                        output.append("You got " + std::to_string(player->GetGold()) + " gold left.\n");
                    }
                    else
                        output = "You can't remove this building, you only got " + std::to_string(player->GetGold()) + " gold.\n";
                }
                else
                    output = "You can't remove this building, because it's a keep.\n";
            }
            else
                output = "This is not a valid choice.\n";

			networkServices->WriteToClient(output, socket, true);
		}

		// Check if the build cards contains a graveyard
		if (chosen_player->ContainsBuildingCard(BuildingEnum::GRAVEYARD))
			chosen_player->GetBuildedBuildingCard(BuildingEnum::GRAVEYARD)->UseCardSpecial(manager, chosen_player);

		output.clear();
		output.append("\n");
		networkServices->WriteToClient(output, socket, true);
	}
	else
	{
		output = "There are no other players available from who you can destroy a building.\n";
	}	

	

	// Receive gold for every build red building
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

	networkServices->WriteToClient("You received " + std::to_string(counter) + " gold.'\n", socket, true);

	// Set used characteristic on true
	player->SetUsedCharacteristic(true);

	// Let the other players know what happend
	if (players_available)
	{
		output.clear();
		output.append(player->GetName() + " destroyed: " + name_destroyed_building + " from " + chosen_player->GetName() + " and " + player->GetName() + " received + " + std::to_string(counter) + " Gold.\n");
		networkServices->WriteToAllClients(output);
	}
	else
	{
		output.clear();
		output.append(player->GetName() + " did not destroy a building and received + " + std::to_string(counter) + " Gold.\n");
		networkServices->WriteToAllClients(output);
	}
}

PlayerCardType Condottiere::GetType()
{
	return PlayerCardType::CONDOTTIERE;
}
