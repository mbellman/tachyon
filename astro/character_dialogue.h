#pragma once

#include "astro/ui_system.h"

#define play_random_dialogue(entity, ...)\
  {\
    std::initializer_list<Dialogue> __messages = __VA_ARGS__;\
    PlayRandomDialogue(tachyon, state, entity, __messages);\
  }\

namespace astro {
  struct Dialogue {
    const char* text = nullptr;
    const char* sound = nullptr;
  };

  typedef std::initializer_list<Dialogue> DialogueList;

  static void PlayRandomDialogue(Tachyon* tachyon, State& state, GameEntity& entity, DialogueList& dialogues) {
    if (!IsSameEntity(entity, state.speaking_entity_record)) {
      // Don't play dialogue if this isn't for the current speaking entity
      // @todo fallback sound? grunt?
      return;
    }

    // Check to see whether we're invoking this on a set of essages
    // which has already been used for the current dialogue. We don't
    // want to rapidly cycle between random dialogue lines, so suppress
    // further dialogue until the current one has cleared.
    //
    // @optimize don't require N messages to be string-compared per invocation
    int total = dialogues.size();

    for (int i = 0; i < total; i++) {
      auto& dialogue = *(dialogues.begin() + i);

      if (state.dialogue_message == dialogue.text) {
        return;
      }
    }

    int index = Tachyon_GetRandom(0, total - 1);
    auto& dialogue = *(dialogues.begin() + index);

    UISystem::ShowDialogue(tachyon, state, dialogue.text);

    if (strcmp(dialogue.sound, "") != 0) {
      Tachyon_PlaySound(dialogue.sound);
    }
  }

  /**
   * ----------------------------
   * BANDIT dialogue lines and voiceover files.
   * @todo should these be loaded in from a file instead?
   * ----------------------------
   */
  static DialogueList bandit_dialogue_stunned_agitated = {
    {
      .text = "Agh! You filthy coward!",
      .sound = ""
    },
    {
      .text = "Wretched little bastard!",
      .sound = ""
    }
  };

  static DialogueList bandit_dialogue_stunned_engaged = {
    {
      .text = "Argh! The bastard blinded me!",
      .sound = "./astro/audio/bandit/blinded.mp3"
    }
  };

  static DialogueList bandit_dialogue_noticed = {
    {
      .text = "Look, we've got one!",
      .sound = "./astro/audio/bandit/got_one.mp3"
    },
    {
      .text = "All by our lonesome, are we?",
      .sound = "./astro/audio/bandit/lonesome.mp3"
    }
  };

  static DialogueList bandit_dialogue_engaged = {
    {
      .text = "I'll make quick work of him!",
      .sound = ""
    },
    {
      .text = "Let's not make this difficult!",
      .sound = ""
    },
    {
      .text = "Oi, where do you think you're going?",
      .sound = ""
    }
  };

  static DialogueList bandit_dialogue_agitated = {
    {
      .text = "Dirty rat! You're in for it now!",
      .sound = ""
    },
    {
      .text = "Dirty rat! You're in for it now!",
      .sound = ""
    },
    {
      .text = "Now you've asked for it!",
      .sound = ""
    }
  };
}